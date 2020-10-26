#include <vmx_modules/syscalls_module.h>
#include <vmx_modules/kpp_module.h>
#include <vmm/msr.h>
#include <debug.h>
#include <win_kernel/memory_manager.h>
#include <intrinsics.h>
#include <vmm/vm_operations.h>
#include <vmm/vmcs.h>

STATUS SyscallsModuleInitializeAllCores(IN PSHARED_CPU_DATA sharedData, IN PMODULE module, IN PGENERIC_MODULE_DATA initData)
{
    PrintDebugLevelDebug("Starting initialization of syscalls module for all cores\n");
    sharedData->heap.allocate(&sharedData->heap, sizeof(SYSCALLS_MODULE_EXTENSION), &module->moduleExtension);
    SetMemory(module->moduleExtension, 0, sizeof(SYSCALLS_MODULE_EXTENSION));
    PSYSCALLS_MODULE_EXTENSION extension = module->moduleExtension;
    extension->kppModule = initData->syscallsModule.kppModule;
    PrintDebugLevelDebug("Shared cores data successfully initialized for syscalls module\n");
    return STATUS_SUCCESS;
}

STATUS SyscallsModuleInitializeSingleCore(IN PSINGLE_CPU_DATA data)
{
    PrintDebugLevelDebug("Starting initialization of syscalls module on core #%d\n", data->coreIdentifier);
    // Hook the event of writing to the LSTAR MSR
    UpdateMsrAccessPolicy(data, MSR_IA32_LSTAR, FALSE, TRUE);
    PrintDebugLevelDebug("Finished initialization of syscalls module on core #%d\n", data->coreIdentifier);
    return STATUS_SUCCESS;
}

STATUS LocateSSDT(IN BYTE_PTR lstar, OUT BYTE_PTR* ssdt)
{
    // Will be updated later
    BYTE pattern[] = { 0x8B, 0xF8, 0xC1, 0xEF, 0x07, 0x83, 0xE7, 0x20, 0x25, 0xFF, 0x0F, 0x00, 0x00 };
    BYTE kernelChunk[13];
    BYTE_PTR patternAddress;
    
    PrintDebugLevelDebug("Starting to search for the pattern: %.b in kernel's address space\n", 13, pattern);
    for(QWORD offset = 0; offset < 0xffffffff; offset++)
    {
        if(CopyGuestMemory(kernelChunk, lstar - offset, 13) != STATUS_SUCCESS)
        {
            Print("Invalid: %8\n", lstar - offset);
        }
        if(!CompareMemory(kernelChunk, pattern, 13))
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

STATUS SyscallsHandleMsrWrite(IN PCURRENT_GUEST_STATE data, IN PMODULE module)
{
    PREGISTERS regs = &data->guestRegisters;
    if(regs->rcx != MSR_IA32_LSTAR)
        return STATUS_VM_EXIT_NOT_HANDLED;
    QWORD msrValue = ((regs->rdx & 0xffffffff) << 32) | (regs->rax & 0xffffffff);
    Print("Guest attempted to write to LSTAR %8 MSR: %8\n", regs->rcx, msrValue);
    __writemsr(MSR_IA32_LSTAR, msrValue);
    regs->rip += vmread(VM_EXIT_INSTRUCTION_LEN);
    BYTE_PTR ssdt;
    if(LocateSSDT(msrValue, &ssdt) != STATUS_SUCCESS)
    {
        Print("Could not find SSDT in kernel's address space\n");
        ASSERT(FALSE);
    }
    return STATUS_SUCCESS;
}