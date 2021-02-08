#include <vmx_modules/module.h>
#include <vmx_modules/kpp_module.h>
#include <vmm/vm_operations.h>
#include <vmm/vmcs.h>
#include <vmm/msr.h>
#include <win_kernel/memory_manager.h>
#include <debug.h>
#include <intrinsics.h>
#include <vmx_modules/hooking_module.h>
#include <vmm/memory_manager.h>
#include <vmm/exit_reasons.h>

STATUS KppModuleInitializeAllCores(IN PMODULE module)
{
    PKPP_MODULE_DATA extension;
    PSHARED_CPU_DATA sharedData;

    sharedData = VmmGetVmmStruct()->currentCPU->sharedData;
    PrintDebugLevelDebug("Starting initialization of KPP module for all cores\n");
    // Init module & name
    MdlInitModule(module);
    MdlSetModuleName(module, KPP_MODULE_NAME);
    // Register VM-Exit handlers
    MdlRegisterVmExitHandler(module, EXIT_REASON_EPT_VIOLATION, KppHandleEptViolation);
    MdlRegisterModule(module);
    // Allocate space for module extension
    sharedData->heap.allocate(&sharedData->heap, sizeof(KPP_MODULE_DATA), &module->moduleExtension);
    // Init extension
    HwSetMemory(module->moduleExtension, 0, sizeof(KPP_MODULE_DATA));
    extension = module->moduleExtension;
    SetInit(&extension->addressSet, BASIC_HASH_LEN, BasicHashFunction);
    ListCreate(&extension->entriesList); 
    PrintDebugLevelDebug("Shared cores data successfully initialized for KPP module\n");
    
    return STATUS_SUCCESS;
}

STATUS KppModuleInitializeSingleCore(PMODULE module)
{
    PSINGLE_CPU_DATA data;

    data = VmmGetVmmStruct()->currentCPU;
    PrintDebugLevelDebug("Starting initialization of KPP module on core #%d\n", data->coreIdentifier);
    PrintDebugLevelDebug("Finished initialization of KPP module on core #%d\n", data->coreIdentifier);
    
    return STATUS_SUCCESS;
}

STATUS KppAddNewEntry(IN QWORD guestPhysicalAddress, IN QWORD hookedInstructionLength, IN BYTE_PTR hookedInstruction)
{
    PKPP_ENTRY_CONTEXT kppContext;
    PHEAP heap;
    static PMODULE module;
    PKPP_MODULE_DATA kppData;
    PSHARED_CPU_DATA shared;
    STATUS status = STATUS_SUCCESS;

    shared = VmmGetVmmStruct()->currentCPU->sharedData;
    if(!module)
        SUCCESS_OR_CLEANUP(MdlGetModuleByName(&module, KPP_MODULE_NAME));
    
    heap = &shared->heap;
    kppData = (PKPP_MODULE_DATA)module->moduleExtension;
    SUCCESS_OR_CLEANUP(heap->allocate(heap, sizeof(KPP_ENTRY_CONTEXT), &kppContext));
    kppContext->hookedInstructionAddress = guestPhysicalAddress;
    kppContext->hookedInstructionLength = hookedInstructionLength;
    HwCopyMemory(kppContext->hookedInstrucion, hookedInstruction, hookedInstructionLength);
    SUCCESS_OR_CLEANUP(ListInsert(&kppData->entriesList, kppContext));
    // Save the address in the addresses set
    SetInsert(&kppData->addressSet, ALIGN_DOWN((QWORD)guestPhysicalAddress, PAGE_SIZE));
    // Mark the page as unreadable & unwritable
    for(QWORD i = 0; i < shared->numberOfCores; i++)
        VmmUpdateEptAccessPolicy(shared->cpuData[i], ALIGN_DOWN((QWORD)guestPhysicalAddress, PAGE_SIZE), 
            PAGE_SIZE, EPT_EXECUTE);

cleanup:
    if(status && kppContext)
        heap->deallocate(heap, kppContext);
    return status;
}

STATUS KppRemoveEntry(IN QWORD guestPhysicalAddress)
{
    PKPP_ENTRY_CONTEXT kppContext;
    PHEAP heap;
    STATUS status;
    static PMODULE module;
    PKPP_MODULE_DATA kppData;
    PSHARED_CPU_DATA shared;
    PLIST_ENTRY listEntry;
    BOOL found;

    if(!module)
        MdlGetModuleByName(&module, KPP_MODULE_NAME);
    heap = &VmmGetVmmStruct()->currentCPU->sharedData->heap;
    kppData = module->moduleExtension;
    // Remove the address from the address set
    SetRemove(&kppData->addressSet, guestPhysicalAddress);
    // Next, find its assosiated KPP entry
    listEntry = kppData->entriesList.head;
    found = FALSE;
    while(listEntry)
    {
        kppContext = (PKPP_ENTRY_CONTEXT)listEntry->data;
        if(kppContext->hookedInstructionAddress == guestPhysicalAddress)
        {
            found = TRUE;
            break;
        }
        listEntry = listEntry->next;
    }
    if(!found)
        return STATUS_UNKNOWN_HOOK_ADDRESS;
    // Remove the context from the list of KPP context data
    Print("Found the entry, removing it from KPP's list...\n");
    ListRemove(&kppData->entriesList, (QWORD)kppContext);
    // Deallocate the memory
    heap->deallocate(heap, kppContext);
    return STATUS_SUCCESS;
}

BOOL KppCheckIfAddressContainsInstruction(IN PKPP_MODULE_DATA kppData, IN QWORD address,
    IN BYTE readLength, OUT BOOL* before, OUT PKPP_ENTRY_CONTEXT* entry, QWORD_PTR ext)
{
    PLIST_ENTRY kppEntry;
    PKPP_ENTRY_CONTEXT kppContext;

    kppEntry = kppData->entriesList.head;
    while(kppEntry)
    {
        kppContext = (PKPP_ENTRY_CONTEXT)kppEntry->data;
        if(kppContext->hookedInstructionAddress <= address
            && ((kppContext->hookedInstructionAddress + kppContext->hookedInstructionLength) >= address))
        {
            *before = FALSE;
            *entry = kppContext;
            *ext = address - kppContext->hookedInstructionAddress;
            return TRUE;
        }
        else if((address <= kppContext->hookedInstructionAddress)
         && (kppContext->hookedInstructionAddress <= (address + readLength)))
        {
            *before = TRUE;
            *entry = kppContext;
            *ext = kppContext->hookedInstructionAddress - address;
            return TRUE;
        }
        kppEntry = kppEntry->next;
    }
    return FALSE;
}

VOID KppBuildResult(OUT PVOID val, IN QWORD guestPhysical, IN QWORD readLength, 
    IN PKPP_MODULE_DATA kppData)
{
    QWORD hostVirtualAddress, ext;
    PKPP_ENTRY_CONTEXT entry;
    BOOL isBefore;

    hostVirtualAddress = WinMmTranslateGuestPhysicalToHostVirtual(guestPhysical);
    if(KppCheckIfAddressContainsInstruction(kppData, guestPhysical, readLength, &isBefore, 
        &entry, &ext))
    {
        // The hidden instruction is a prefix of the current checked instruction
        // OR the hidden instruction contains all of the checked instruction
        if(!isBefore)
        {
            // ext = offset from the beggining of the hidden instruction
            HwCopyMemory(val, entry->hookedInstrucion + ext, entry->hookedInstructionLength - ext 
                <= readLength ? entry->hookedInstructionLength - ext : readLength);
            if(entry->hookedInstructionLength - ext < readLength)
                HwCopyMemory((BYTE_PTR)val + (entry->hookedInstructionLength - ext), 
                    WinMmTranslateGuestPhysicalToHostVirtual(guestPhysical + entry->hookedInstructionLength 
                        - ext), readLength - (entry->hookedInstructionLength - ext));
        }
        // The hidden instruction is a suffix of the current checked instruction
        // OR the checked instruction contains the hidden instruction 
        else
        {
            // ext = the number of bytes before the beggining of the hidden instruction
            // Copy the beginning
            HwCopyMemory(val, hostVirtualAddress, ext);
            // Copy the replaced instruction itself
            if(readLength > ext)
            {
                HwCopyMemory((BYTE_PTR)val + ext, entry->hookedInstrucion, readLength - ext 
                    <= entry->hookedInstructionLength ? readLength - ext : entry->hookedInstructionLength);
                // Copy the rest of the instruction, if we need to
                if(readLength - ext > entry->hookedInstructionLength)
                    HwCopyMemory((BYTE_PTR)val + ext + entry->hookedInstructionLength, 
                                hostVirtualAddress + ext + entry->hookedInstructionLength,
                                readLength - ext - entry->hookedInstructionLength);
            }
        }      
    }
    else
        HwCopyMemory(val, hostVirtualAddress, readLength);
}

STATUS KppEmulatePatchGuardAction(IN PKPP_MODULE_DATA kppData, IN QWORD address, IN BYTE instructionLength)
{
    PREGISTERS regs;
    BYTE inst[X86_MAX_INSTRUCTION_LEN];

    regs = &VmmGetVmmStruct()->guestRegisters;
    WinMmCopyGuestMemory(inst, regs->rip, instructionLength);
#ifdef DEBUG_KPP_COMMANDS
    Print("Running at (RIP)%8, referencing (PA)%8, %.b\n", regs->rip, address, instructionLength, inst);
#endif
    if(instructionLength == 8 && inst[0] == 0x41 && inst[1] == 0x8b && inst[2] == 0x94 && 
        inst[3] == 0x80 && inst[4] == 0x90 && inst[5] == 0x5b && inst[6] == 0x5e && 
        inst[7] == 0x00)
    {
        // mov edx,DWORD PTR [r8+rax*4+0x5e5b90]
        DWORD val;
        KppBuildResult(&val, address, 4, kppData);
        regs->rdx = val;
    }
    else if(instructionLength == 9 && inst[0] == 0x41 && inst[1] == 0x0f && inst[2] == 0xb6 && 
        inst[3] == 0x84 && inst[4] == 0x00 && inst[5] == 0xa0 && inst[6] == 0x5b &&
        inst[7] == 0x5e && inst[8] == 0x00)
    {
        // movzx eax,BYTE PTR [r8+rax*1+0x5e5ba0]
        BYTE val;
        KppBuildResult(&val, address, 1, kppData);
        regs->rax = (regs->rax & 0xffffffff00000000ULL) | val;
    }
    else if(instructionLength == 3 && inst[0] == 0x41 && inst[1] == 0x8b && inst[2] == 0x02)
    {
        // mov eax,DWORD PTR [r10]
        DWORD val;
        KppBuildResult(&val, address, 4, kppData);
        regs->rax = val;
    }
    else if((instructionLength == 3 && inst[0] == 0x41 && inst[1] == 0x8a && inst[2] == 0x02) ||
        (instructionLength == 2 && inst[0] == 0x8a && inst[1] == 0x00))
    {
        // mov al,BYTE PTR [r10]
        BYTE val;
        KppBuildResult(&val, address, 1, kppData);
        regs->rax = (regs->rax & 0xffffffffffffff00ULL) | val;
    }
    else if(instructionLength == 4 && inst[0] == 0x41 && inst[1] == 0x0f && inst[2] == 0xb7 && inst[3] == 0x02)
    {
        // movzx eax,WORD PTR [r10]
        WORD val;
        KppBuildResult(&val, address, 2, kppData);
        regs->rax = (regs->rax & 0xffffffff00000000ULL) | val;
    }
    else if(instructionLength == 3 && inst[0] == 0x49 && inst[1] == 0x8b && inst[2] == 0x01)
    {
        // mov rax,QWORD PTR [r9]
        QWORD val;
        KppBuildResult(&val, address, 8, kppData);
        regs->rax = val;
    }
    else if((instructionLength == 3 && inst[0] == 0x49 && inst[1] == 0x33 && inst[2] == 0x18)
        || (instructionLength == 4 && inst[0] == 0x49 && inst[1] == 0x33 && inst[2] == 0x58 && inst[3] == 0x8)
        || (instructionLength == 3 && inst[0] == 0x48 && inst[1] == 0x33 && inst[2] == 0x1f)
        || (instructionLength == 3 && inst[0] == 0x49 && inst[1] == 0x33 && inst[2] == 0x19))
    {
        // xor rbx,QWORD PTR [r8]
        // xor rbx,QWORD PTR [r8+0x8]
        // xor rbx,QWORD PTR [rdi]
        // xor rbx,QWORD PTR [r9]
        QWORD val;
        KppBuildResult(&val, address, 8, kppData);
        regs->rbx ^= val;
    }
    else if((instructionLength == 4 && inst[0] == 0x41 && inst[1] == 0x0f && inst[2] == 0xb6 && inst[3] == 0x00)
        || (instructionLength == 4 && inst[0] == 0x41 && inst[1] == 0x0f && inst[2] == 0xb6 && inst[3] == 0x01))
    {
        // movzx eax,BYTE PTR [r8]
        // movzx eax,BYTE PTR [r9]
        BYTE val;
        KppBuildResult(&val, address, 1, kppData);
        regs->rax = (regs->rax & 0xffffffff00000000) | val;
    }
    else if(instructionLength == 4 && inst[0] == 0x4d && inst[1] == 0x8b && inst[2] == 0x41 && inst[3] == 0x08)
    {
        // mov r8,QWORD PTR [r9+0x8]
        QWORD val;
        KppBuildResult(&val, address, 8, kppData);
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
        KppBuildResult(&val, address, 8, kppData);
        regs->r8 ^= val;
    }
    else if(instructionLength == 4 && inst[0] == 0xc5 && inst[1] == 0xfe && inst[2] == 0x6f && inst[3] == 0x00)
    {
        // vmovdqu ymm0,YMMWORD PTR [rax]
        BYTE val[16];
        KppBuildResult(&val, address, 16, kppData);
        __vmovdqu_ymm0(val);
    }
    else if(instructionLength == 4 && inst[0] == 0x49 && inst[1] == 0x8b && inst[2] == 0x59 && inst[3] == 0x08)
    {
        // mov rbx,QWORD PTR [r9+0x8]
        QWORD val;
        KppBuildResult(&val, address, 8, kppData);
        regs->rbx = val;
    }
    else if(instructionLength == 4 && inst[0] == 0x0f && inst[1] == 0x10 && inst[2] == 0x04 && inst[3] == 0x11)
    {
        // movups xmm0, XMMWORD PTR [rcx+rdx*1]
        BYTE val[16];
        KppBuildResult(&val, address, 16, kppData);
        __movups_xmm0(val);
    }
    // Kernel-debugging area
    else if(instructionLength == 3 && inst[0] == 0x41 && inst[1] == 0x88 && inst[2] == 0x02)
    {
        // mov BYTE PTR [r10],al
        BYTE_PTR hostVirtual = WinMmTranslateGuestPhysicalToHostVirtual(address);
        *hostVirtual = (BYTE)(regs->rax & 0xff);
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
    QWORD address, instructionLength;
    PKPP_MODULE_DATA kppData;

    address = vmread(GUEST_PHYSICAL_ADDRESS);
    instructionLength = vmread(VM_EXIT_INSTRUCTION_LEN);
    kppData = (PKPP_MODULE_DATA)module->moduleExtension;
    if(!IsInSet(&kppData->addressSet, ALIGN_DOWN(address, PAGE_SIZE)))
        return STATUS_VM_EXIT_NOT_HANDLED;
    return KppEmulatePatchGuardAction(kppData, address, instructionLength);
}

REGISTER_MODULE(KppModuleInitializeAllCores, KppModuleInitializeSingleCore, kpp);