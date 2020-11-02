#include <vmx_modules/module.h>
#include <vmx_modules/kpp_module.h>
#include <vmm/vm_operations.h>
#include <vmm/vmcs.h>
#include <vmm/msr.h>
#include <win_kernel/memory_manager.h>
#include <debug.h>

STATUS KppModuleInitializeAllCores(IN PSHARED_CPU_DATA sharedData, IN PMODULE module, IN GENERIC_MODULE_DATA initData)
{
    PrintDebugLevelDebug("Starting initialization of KPP module for all cores\n");
    sharedData->heap.allocate(&sharedData->heap, sizeof(KPP_MODULE_DATA), &module->moduleExtension);
    SetMemory(module->moduleExtension, 0, sizeof(KPP_MODULE_DATA));
    PKPP_MODULE_DATA extension = module->moduleExtension;
    extension->syscallsData = &__ntDataStart;
    PrintDebugLevelDebug("Shared cores data successfully initialized for KPP module\n");
    return STATUS_SUCCESS;
}

STATUS KppModuleInitializeSingleCore(IN PSINGLE_CPU_DATA data)
{
    PrintDebugLevelDebug("Starting initialization of KPP module on core #%d\n", data->coreIdentifier);
    PrintDebugLevelDebug("Finished initialization of KPP module on core #%d\n", data->coreIdentifier);
    return STATUS_SUCCESS;
}

STATUS RegisterNewProtectedKppEntry(IN QWORD syscall, IN QWORD guestPhysicalAddress, IN BYTE_PTR instruction, 
    IN BYTE instructionLength, IN BOOL hookReturn, IN PMODULE kppModule)
{
    PKPP_MODULE_DATA kppData = (PKPP_MODULE_DATA)kppModule->moduleExtension;
    if(kppData->hookedSyscallsCount >= KPP_MODULE_MAX_COUNT)
        return STATUS_NO_SPACE_AVAILABLE;
    QWORD idx;
    BOOL exist = FALSE;
    for(QWORD i = 0; i < kppData->hookedSyscallsCount; i++)
    {
        if(kppData->hookedSyscalls[i] == syscall)
        {
            exist = TRUE;
            idx = i;
            break;
        }
    }
    if(!exist)
    {
        kppData->hookedSyscalls[kppData->hookedSyscallsCount++] = syscall;
        idx = kppData->hookedSyscallsCount;
    }
    if(hookReturn)
    {
        kppData->syscallsData[kppData->hookedSyscalls[idx]].returnHookAddress = guestPhysicalAddress;
        return STATUS_SUCCESS;
    }
    kppData->syscallsData[kppData->hookedSyscalls[idx]].hookedInstructionAddress = guestPhysicalAddress;
    kppData->syscallsData[kppData->hookedSyscalls[idx]].hookedInstructionLength = instructionLength;
    CopyMemory(kppData->syscallsData[kppData->hookedSyscalls[idx]].hookedInstrucion, instruction, 
        instructionLength);
    return STATUS_SUCCESS;
}

BOOL CheckIfAddressContainsInstruction(IN PKPP_MODULE_DATA kppData, IN QWORD address,
    IN BYTE readLength, OUT BOOL* before, OUT PSYSCALL_DATA* entry, QWORD_PTR ext)
{
    for(QWORD i = 0; i < kppData->hookedSyscallsCount; i++)
    {
        if(address >= kppData->syscallsData[kppData->hookedSyscalls[i]].hookedInstructionAddress
             && address <= kppData->syscallsData[kppData->hookedSyscalls[i]].hookedInstructionAddress
              + kppData->syscallsData[kppData->hookedSyscalls[i]].hookedInstructionLength)
        {
            *before = FALSE;
            *entry = &(kppData->syscallsData[kppData->hookedSyscalls[i]]);
            *ext = address - kppData->syscallsData[kppData->hookedSyscalls[i]].hookedInstructionAddress;
            return TRUE;
        }
        else if(address <= kppData->syscallsData[kppData->hookedSyscalls[i]].hookedInstructionAddress
         && address + readLength >= kppData->syscallsData[kppData->hookedSyscalls[i]].hookedInstructionAddress)
        {
            *before = TRUE;
            *entry = &(kppData->syscallsData[kppData->hookedSyscalls[i]]);
            *ext = kppData->syscallsData[kppData->hookedSyscalls[i]].hookedInstructionAddress - address;
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
    else if(instructionLength == 3 && inst[0] == 0x41 && inst[1] == 0x8a && inst[2] == 0x02)
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
    return EmulatePatchGuardAction(kppData, address, instructionLength);
}