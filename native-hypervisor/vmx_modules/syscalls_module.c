#include <vmx_modules/syscalls_module.h>
#include <vmm/msr.h>
#include <debug.h>

STATUS SyscallsModuleInitialize(IN PSHARED_CPU_DATA sharedData, IN PMODULE module)
{
    PrintDebugLevelDebug("Initializing syscalls module\n");
    UpdateMsrAccessPolicy(sharedData, MSR_IA32_LSTAR, FALSE, TRUE);
    PrintDebugLevelDebug("Syscalls module successfully initialized\n");
    return STATUS_SUCCESS;
}

STATUS SyscallsHandleMsrWrite(IN PCURRENT_GUEST_STATE data)
{
    PREGISTERS regs = &data->guestRegisters;
    Print("Guest attempted to write to %8 MSR\n", regs->rcx);
    ASSERT(FALSE);
    return STATUS_SUCCESS;
}