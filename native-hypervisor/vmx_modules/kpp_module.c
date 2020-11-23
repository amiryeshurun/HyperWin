#include <vmx_modules/module.h>
#include <vmx_modules/kpp_module.h>
#include <vmm/vm_operations.h>
#include <vmm/vmcs.h>
#include <vmm/msr.h>
#include <win_kernel/memory_manager.h>
#include <debug.h>
#include <intrinsics.h>
#include <vmx_modules/syscalls_module.h>

STATUS KppModuleInitializeAllCores(IN PSHARED_CPU_DATA sharedData, IN PMODULE module, IN PGENERIC_MODULE_DATA initData)
{
    PrintDebugLevelDebug("Starting initialization of KPP module for all cores\n");
    sharedData->heap.allocate(&sharedData->heap, sizeof(KPP_MODULE_DATA), &module->moduleExtension);
    SetMemory(module->moduleExtension, 0, sizeof(KPP_MODULE_DATA));
    PKPP_MODULE_DATA extension = module->moduleExtension;
    extension->syscallsData = &__ntDataStart;
    PSYSCALLS_MODULE_EXTENSION syscallsExt = initData->kppModule.syscallsModule->moduleExtension;
    extension->syscallsMap = &syscallsExt->addressToSyscall;
    extension->addressSet = &syscallsExt->addressSet;
    PrintDebugLevelDebug("Shared cores data successfully initialized for KPP module\n");
    return STATUS_SUCCESS;
}

STATUS KppModuleInitializeSingleCore(IN PSINGLE_CPU_DATA data)
{
    PrintDebugLevelDebug("Starting initialization of KPP module on core #%d\n", data->coreIdentifier);
    PrintDebugLevelDebug("Finished initialization of KPP module on core #%d\n", data->coreIdentifier);
    return STATUS_SUCCESS;
}

BOOL CheckIfAddressContainsInstruction(IN PKPP_MODULE_DATA kppData, IN QWORD address,
    IN BYTE readLength, OUT BOOL* before, OUT PSYSCALL_DATA* entry, QWORD_PTR ext)
{
    QWORD hookedSyscallsCount, hookedSyscalls[KPP_MODULE_MAX_COUNT];
    MapGetValues(kppData->syscallsMap, hookedSyscalls, &hookedSyscallsCount);
    for(QWORD i = 0; i < hookedSyscallsCount; i++)
    {
        if(address >= kppData->syscallsData[hookedSyscalls[i]].hookedInstructionAddress
             && address <= kppData->syscallsData[hookedSyscalls[i]].hookedInstructionAddress
              + kppData->syscallsData[hookedSyscalls[i]].hookedInstructionLength)
        {
            *before = FALSE;
            *entry = &(kppData->syscallsData[hookedSyscalls[i]]);
            *ext = address - kppData->syscallsData[hookedSyscalls[i]].hookedInstructionAddress;
            return TRUE;
        }
        else if(address <= kppData->syscallsData[hookedSyscalls[i]].hookedInstructionAddress
         && address + readLength >= kppData->syscallsData[hookedSyscalls[i]].hookedInstructionAddress)
        {
            *before = TRUE;
            *entry = &(kppData->syscallsData[hookedSyscalls[i]]);
            *ext = kppData->syscallsData[hookedSyscalls[i]].hookedInstructionAddress - address;
            return TRUE;
        }
    }
    
    return FALSE;
}

VOID BuildKppResult(OUT PVOID val, IN QWORD guestPhysical, IN QWORD readLength, 
    IN PKPP_MODULE_DATA kppData)
{
    QWORD hostVirtualAddress = TranslateGuestPhysicalToHostVirtual(guestPhysical), ext;
    PSYSCALL_DATA entry;
    BOOL isBefore;
    if(CheckIfAddressContainsInstruction(kppData, guestPhysical, readLength, &isBefore, 
        &entry, &ext))
    {
        // The hidden instruction is a prefix of the current checked instruction
        if(!isBefore)
        {
            // ext = offset from the beggining of the hidden instruction
            CopyMemory(val, entry->hookedInstrucion + ext, entry->hookedInstructionLength - ext 
                <= readLength ? entry->hookedInstructionLength - ext : readLength);
            if(entry->hookedInstructionLength - ext < readLength)
                CopyMemory((BYTE_PTR)val + (entry->hookedInstructionLength - ext), 
                    TranslateGuestPhysicalToHostVirtual(guestPhysical + entry->hookedInstructionLength 
                        - ext), readLength - (entry->hookedInstructionLength - ext));
        }
        // The hidden instruction is a suffix of the current checked instruction
        else
        {
            // ext = the number of bytes before the beggining of the hidden instruction
            CopyMemory(val, hostVirtualAddress, ext);
            CopyMemory((BYTE_PTR)val + ext, entry->hookedInstrucion, readLength - ext);
        }      
    }
    else
        CopyMemory(val, hostVirtualAddress, readLength);
}

STATUS EmulatePatchGuardAction(IN PKPP_MODULE_DATA kppData, IN QWORD address, IN BYTE instructionLength)
{
    PREGISTERS regs = &(GetVMMStruct()->guestRegisters);
    BYTE inst[X86_MAX_INSTRUCTION_LEN];
    BOOL isProtectedAddress, isBefore;
    QWORD ext;
    PSYSCALL_DATA entry;

    CopyGuestMemory(inst, regs->rip, instructionLength);
    if(instructionLength == 3 && inst[0] == 0x41 && inst[1] == 0x8b && inst[2] == 0x02)
    {
        // mov eax,DWORD PTR [r10]
        DWORD val;
        BuildKppResult(&val, address, 4, kppData);
        regs->rax = (regs->rax & 0xffffffff00000000ULL) | val;
    }
    else if((instructionLength == 3 && inst[0] == 0x41 && inst[1] == 0x8a && inst[2] == 0x02) ||
        (instructionLength == 2 && inst[0] == 0x8a && inst[1] == 0x00))
    {
        // mov al,BYTE PTR [r10]
        BYTE val;
        BuildKppResult(&val, address, 1, kppData);
        regs->rax = (regs->rax & 0xffffffffffffff00ULL) | val;
    }
    else if(instructionLength == 4 && inst[0] == 0x41 && inst[1] == 0x0f && inst[2] == 0xb7 && inst[3] == 0x02)
    {
        // movzx eax,WORD PTR [r10]
        WORD val;
        BuildKppResult(&val, address, 2, kppData);
        regs->rax = (regs->rax & 0xffffffff00000000ULL) | val;
    }
    else if(instructionLength == 3 && inst[0] == 0x49 && inst[1] == 0x8b && inst[2] == 0x01)
    {
        // mov rax,QWORD PTR [r9]
        QWORD val;
        BuildKppResult(&val, address, 8, kppData);
        regs->rax = val;
    }
    else if((instructionLength == 3 && inst[0] == 0x49 && inst[1] == 0x33 && inst[2] == 0x18)
        || (instructionLength == 4 && inst[0] == 0x49 && inst[1] == 0x33 && inst[2] == 0x58 && inst[3] == 0x8))
    {
        // xor rbx,QWORD PTR [r8]
        // xor rbx,QWORD PTR [r8+0x8]
        QWORD val;
        BuildKppResult(&val, address, 8, kppData);
        regs->rbx ^= val;
    }
    else if((instructionLength == 4 && inst[0] == 0x41 && inst[1] == 0x0f && inst[2] == 0xb6 && inst[3] == 0x00)
        || (instructionLength == 4 && inst[0] == 0x41 && inst[1] == 0x0f && inst[2] == 0xb6 && inst[3] == 0x01))
    {
        // movzx eax,BYTE PTR [r8]
        // movzx eax,BYTE PTR [r9]
        BYTE val;
        BuildKppResult(&val, address, 1, kppData);
        regs->rax = (regs->rax & 0xffffffff00000000) | val;
    }
    else if(instructionLength == 4 && inst[0] == 0x4d && inst[1] == 0x8b && inst[2] == 0x41 && inst[3] == 0x08)
    {
        // mov r8,QWORD PTR [r9+0x8]
        QWORD val;
        BuildKppResult(&val, address, 8, kppData);
        regs->r8 = val;
    }
    else if((instructionLength == 3 && inst[0] == 0x4d && inst[1] == 0x33 && inst[2] == 0x01)
        || (instructionLength == 3 && inst[0] == 0x4d && inst[1] == 0x33 && inst[2] == 0x02)
        || (instructionLength == 4 && inst[0] == 0x4d && inst[1] == 0x33 && inst[2] == 0x42 && inst[3] == 0x08))
    {
        // xor r8,QWORD PTR [r9]
        // xor r8,QWORD PTR [r10]
        // xor r8,QWORD PTR [r10+0x8]
        QWORD val;
        BuildKppResult(&val, address, 8, kppData);
        regs->r8 ^= val;
    }
    else if(instructionLength == 4 && inst[0] == 0xc5 && inst[1] == 0xfe && inst[2] == 0x6f && inst[3] == 0x00)
    {
        // vmovdqu ymm0,YMMWORD PTR [rax]
        BYTE val[16];
        BuildKppResult(&val, address, 16, kppData);
        __vmovdqu_ymm0(val);
    }
    else if(instructionLength == 3 && inst[0] == 0x48 && inst[1] == 0x33 && inst[2] == 0x1f)
    {
        // xor rbx,QWORD PTR [rdi]
        QWORD val;
        BuildKppResult(&val, address, 8, kppData);
        regs->rbx ^= val;
    }
    else
    {
        Print("New KPP instruction found at %8: %.b\n", address, instructionLength, inst);
        return STATUS_UNKNOWN_KPP_INSTRUCTION;
    }
    regs->rip += instructionLength;
    return STATUS_SUCCESS;
}

STATUS KppHandleEptViolation(IN PCURRENT_GUEST_STATE data, IN PMODULE module)
{
    QWORD address = vmread(GUEST_PHYSICAL_ADDRESS), instructionLength = vmread(VM_EXIT_INSTRUCTION_LEN);
    PKPP_MODULE_DATA kppData = (PKPP_MODULE_DATA)module->moduleExtension;
    if(!IsInSet(kppData->addressSet, ALIGN_DOWN(address, PAGE_SIZE)))
        return STATUS_VM_EXIT_NOT_HANDLED;
    return EmulatePatchGuardAction(kppData, address, instructionLength);
}