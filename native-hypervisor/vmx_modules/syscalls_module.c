#include <vmx_modules/syscalls_module.h>
#include <vmm/msr.h>
#include <debug.h>
#include <win_kernel/memory_manager.h>

STATUS SyscallsModuleInitialize(IN PSHARED_CPU_DATA sharedData, IN PMODULE module, 
    IN PGENERIC_MODULE_DATA initData)
{
    PrintDebugLevelDebug("Initializing syscalls module\n");
    sharedData->heap.allocate(&sharedData->heap, sizeof(SYSCALLS_MODULE_EXTENSION), &module->moduleExtension);
    SetMemory(module->moduleExtension, 0, sizeof(SYSCALLS_MODULE_EXTENSION));
    (PSYSCALLS_MODULE_EXTENSION)(module->moduleExtension)->kppModule = initData->syscallsModule.kppModule;
    // Hook the event of writing to LSTAR MSR by OS
    UpdateMsrAccessPolicy(sharedData, MSR_IA32_LSTAR, FALSE, TRUE);
    PrintDebugLevelDebug("Syscalls module successfully initialized\n");
    return STATUS_SUCCESS;
}

STATUS LocateSSDT(IN BYTE_PTR lstar, OUT BYTE_PTR ssdt)
{
    // Will be updated later
    BYTE pattern[] = { 0x00, 0x00 };
    BYTE kernelChunk[15];
    BYTE_PTR patternAddress;
    
    PrintDebugLevelDebug("Starting to search for the pattern: %.b in kernel's address space\n", 15, pattern);
    for(QWORD offset = 0; offset < 0xffffffff; offset++)
    {
        CopyGuestMemory(kernelChunk, lstar - offset, 15);
        if(!CompareMemory(kernelChunk, pattern, 15))
        {
            Print("Pattern found in kernel's address space\n");
            patternAddress = lstar - offset;
            goto SSDTFound;
        }
    }
    Print("Pattern was NOT found in kernel's address space\n");
    return STATUS_SSDT_NOT_FOUND;

SSDTFound:

    return STATUS_SUCCESS;
}

STATUS SyscallsHandleMsrWrite(IN PCURRENT_GUEST_STATE data)
{
    PREGISTERS regs = &data->guestRegisters;
    if(regs->rcx != MSR_IA32_LSTAR)
        return STATUS_VM_EXIT_NOT_HANDLED;
    Print("Guest attempted to write to %8 MSR\n", regs->rcx);
    QWORD msrValue = ((regs->rdx & 0xffffffff) << 32) | (regs->rax & 0xffffffff);
    BYTE_PTR ssdt;
    if(LocateSSDT() != STATUS_SUCCESS)
    {
        Print("Could not find SSDT in kernel's address space\n");
        ASSERT(FALSE);
    }
    return STATUS_SUCCESS;
}