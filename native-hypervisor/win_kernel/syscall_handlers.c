#include <win_kernel/syscall_handlers.h>
#include <win_kernel/memory_manager.h>
#include <debug.h>
#include <vmm/vmcs.h>
#include <vmm/vm_operations.h>

static __attribute__((section(".nt_data"))) SYSCALL_DATA syscallsData[] = {  { NULL, 8 },  { NULL, 1 },  { NULL, 6 },  { NULL, 3 },  { NULL, 3 },  { NULL, 3 },  { NULL, 9 },  { NULL, 10 }, 
 { NULL, 9 },  { NULL, 5 },  { NULL, 3 },  { NULL, 4 },  { NULL, 2 },  { NULL, 4 },  { NULL, 2 },  { NULL, 1 }, 
 { NULL, 5 },  { NULL, 5 },  { NULL, 3 },  { NULL, 6 },  { NULL, 3 },  { NULL, 2 },  { NULL, 5 },  { NULL, 6 }, 
 { NULL, 6 },  { NULL, 5 },  { NULL, 5 },  { NULL, 9 },  { NULL, 4 },  { NULL, 7 },  { NULL, 4 },  { NULL, 2 }, 
 { NULL, 2 },  { NULL, 5 },  { NULL, 3 },  { NULL, 6 },  { NULL, 4 },  { NULL, 5 },  { NULL, 4 },  { NULL, 5 }, 
 { NULL, 10 },  { NULL, 11 },  { NULL, 2 },  { NULL, 5 },  { NULL, 2 },  { NULL, 1 },  { NULL, 9 },  { NULL, 5 }, 
 { NULL, 4 },  { NULL, 2 },  { NULL, 6 },  { NULL, 6 },  { NULL, 2 },  { NULL, 11 },  { NULL, 4 },  { NULL, 3 }, 
 { NULL, 5 },  { NULL, 10 },  { NULL, 5 },  { NULL, 3 },  { NULL, 7 },  { NULL, 2 },  { NULL, 1 },  { NULL, 5 }, 
 { NULL, 3 },  { NULL, 6 },  { NULL, 6 },  { NULL, 2 },  { NULL, 1 },  { NULL, 5 },  { NULL, 0 },  { NULL, 3 }, 
 { NULL, 5 },  { NULL, 5 },  { NULL, 7 },  { NULL, 2 },  { NULL, 2 },  { NULL, 9 },  { NULL, 8 },  { NULL, 2 }, 
 { NULL, 5 },  { NULL, 5 },  { NULL, 2 },  { NULL, 2 },  { NULL, 6 },  { NULL, 11 },  { NULL, 5 },  { NULL, 6 }, 
 { NULL, 3 },  { NULL, 16 },  { NULL, 1 },  { NULL, 5 },  { NULL, 4 },  { NULL, 2 },  { NULL, 4 },  { NULL, 5 }, 
 { NULL, 6 },  { NULL, 2 },  { NULL, 7 },  { NULL, 11 },  { NULL, 11 },  { NULL, 16 },  { NULL, 17 },  { NULL, 3 }, 
 { NULL, 4 },  { NULL, 2 },  { NULL, 2 },  { NULL, 6 },  { NULL, 16 },  { NULL, 2 },  { NULL, 1 },  { NULL, 1 }, 
 { NULL, 1 },  { NULL, 3 },  { NULL, 3 },  { NULL, 4 },  { NULL, 7 },  { NULL, 9 },  { NULL, 3 },  { NULL, 11 }, 
 { NULL, 11 },  { NULL, 3 },  { NULL, 6 },  { NULL, 4 },  { NULL, 3 },  { NULL, 3 },  { NULL, 3 },  { NULL, 3 }, 
 { NULL, 3 },  { NULL, 3 },  { NULL, 2 },  { NULL, 3 },  { NULL, 3 },  { NULL, 6 },  { NULL, 6 },  { NULL, 5 }, 
 { NULL, 6 },  { NULL, 3 },  { NULL, 8 },  { NULL, 4 },  { NULL, 2 },  { NULL, 2 },  { NULL, 8 },  { NULL, 4 }, 
 { NULL, 3 },  { NULL, 3 },  { NULL, 2 },  { NULL, 2 },  { NULL, 2 },  { NULL, 2 },  { NULL, 2 },  { NULL, 2 }, 
 { NULL, 2 },  { NULL, 2 },  { NULL, 2 },  { NULL, 3 },  { NULL, 1 },  { NULL, 1 },  { NULL, 8 },  { NULL, 4 }, 
 { NULL, 4 },  { NULL, 3 },  { NULL, 5 },  { NULL, 9 },  { NULL, 8 },  { NULL, 3 },  { NULL, 3 },  { NULL, 4 }, 
 { NULL, 3 },  { NULL, 3 },  { NULL, 8 },  { NULL, 4 },  { NULL, 9 },  { NULL, 8 },  { NULL, 4 },  { NULL, 14 }, 
 { NULL, 4 },  { NULL, 4 },  { NULL, 5 },  { NULL, 4 },  { NULL, 8 },  { NULL, 9 },  { NULL, 10 },  { NULL, 4 }, 
 { NULL, 7 },  { NULL, 5 },  { NULL, 4 },  { NULL, 11 },  { NULL, 4 },  { NULL, 5 },  { NULL, 13 },  { NULL, 17 }, 
 { NULL, 10 },  { NULL, 6 },  { NULL, 11 },  { NULL, 3 },  { NULL, 5 },  { NULL, 7 },  { NULL, 10 },  { NULL, 2 }, 
 { NULL, 3 },  { NULL, 1 },  { NULL, 1 },  { NULL, 1 },  { NULL, 1 },  { NULL, 1 },  { NULL, 3 },  { NULL, 1 }, 
 { NULL, 2 },  { NULL, 2 },  { NULL, 1 },  { NULL, 0 },  { NULL, 1 },  { NULL, 1 },  { NULL, 0 },  { NULL, 2 }, 
 { NULL, 2 },  { NULL, 3 },  { NULL, 5 },  { NULL, 2 },  { NULL, 5 },  { NULL, 6 },  { NULL, 14 },  { NULL, 5 }, 
 { NULL, 2 },  { NULL, 3 },  { NULL, 1 },  { NULL, 0 },  { NULL, 4 },  { NULL, 0 },  { NULL, 3 },  { NULL, 1 }, 
 { NULL, 2 },  { NULL, 6 },  { NULL, 6 },  { NULL, 2 },  { NULL, 0 },  { NULL, 1 },  { NULL, 2 },  { NULL, 3 }, 
 { NULL, 5 },  { NULL, 6 },  { NULL, 5 },  { NULL, 7 },  { NULL, 7 },  { NULL, 1 },  { NULL, 3 },  { NULL, 5 }, 
 { NULL, 3 },  { NULL, 1 },  { NULL, 4 },  { NULL, 0 },  { NULL, 0 },  { NULL, 2 },  { NULL, 1 },  { NULL, 9 }, 
 { NULL, 2 },  { NULL, 3 },  { NULL, 8 },  { NULL, 10 },  { NULL, 2 },  { NULL, 1 },  { NULL, 4 },  { NULL, 1 }, 
 { NULL, 1 },  { NULL, 5 },  { NULL, 6 },  { NULL, 3 },  { NULL, 9 },  { NULL, 1 },  { NULL, 1 },  { NULL, 9 }, 
 { NULL, 10 },  { NULL, 10 },  { NULL, 12 },  { NULL, 8 },  { NULL, 5 },  { NULL, 3 },  { NULL, 3 },  { NULL, 3 }, 
 { NULL, 4 },  { NULL, 4 },  { NULL, 5 },  { NULL, 3 },  { NULL, 3 },  { NULL, 12 },  { NULL, 3 },  { NULL, 4 }, 
 { NULL, 3 },  { NULL, 3 },  { NULL, 5 },  { NULL, 3 },  { NULL, 3 },  { NULL, 3 },  { NULL, 4 },  { NULL, 3 }, 
 { NULL, 5 },  { NULL, 6 },  { NULL, 3 },  { NULL, 2 },  { NULL, 2 },  { NULL, 2 },  { NULL, 2 },  { NULL, 3 }, 
 { NULL, 6 },  { NULL, 5 },  { NULL, 4 },  { NULL, 3 },  { NULL, 2 },  { NULL, 1 },  { NULL, 2 },  { NULL, 2 }, 
 { NULL, 2 },  { NULL, 10 },  { NULL, 7 },  { NULL, 2 },  { NULL, 9 },  { NULL, 2 },  { NULL, 5 },  { NULL, 5 }, 
 { NULL, 5 },  { NULL, 5 },  { NULL, 5 },  { NULL, 5 },  { NULL, 5 },  { NULL, 5 },  { NULL, 5 },  { NULL, 1 }, 
 { NULL, 2 },  { NULL, 5 },  { NULL, 5 },  { NULL, 6 },  { NULL, 5 },  { NULL, 2 },  { NULL, 4 },  { NULL, 0 }, 
 { NULL, 9 },  { NULL, 6 },  { NULL, 5 },  { NULL, 6 },  { NULL, 5 },  { NULL, 3 },  { NULL, 4 },  { NULL, 5 }, 
 { NULL, 6 },  { NULL, 3 },  { NULL, 6 },  { NULL, 5 },  { NULL, 6 },  { NULL, 3 },  { NULL, 6 },  { NULL, 2 }, 
 { NULL, 2 },  { NULL, 1 },  { NULL, 1 },  { NULL, 5 },  { NULL, 1 },  { NULL, 4 },  { NULL, 1 },  { NULL, 6 }, 
 { NULL, 2 },  { NULL, 2 },  { NULL, 2 },  { NULL, 3 },  { NULL, 3 },  { NULL, 2 },  { NULL, 2 },  { NULL, 2 }, 
 { NULL, 3 },  { NULL, 3 },  { NULL, 1 },  { NULL, 0 },  { NULL, 2 },  { NULL, 2 },  { NULL, 2 },  { NULL, 2 }, 
 { NULL, 2 },  { NULL, 2 },  { NULL, 3 },  { NULL, 3 },  { NULL, 9 },  { NULL, 0 },  { NULL, 2 },  { NULL, 2 }, 
 { NULL, 5 },  { NULL, 6 },  { NULL, 2 },  { NULL, 3 },  { NULL, 1 },  { NULL, 2 },  { NULL, 1 },  { NULL, 2 }, 
 { NULL, 4 },  { NULL, 1 },  { NULL, 1 },  { NULL, 2 },  { NULL, 5 },  { NULL, 4 },  { NULL, 4 },  { NULL, 4 }, 
 { NULL, 4 },  { NULL, 4 },  { NULL, 4 },  { NULL, 4 },  { NULL, 4 },  { NULL, 6 },  { NULL, 4 },  { NULL, 2 }, 
 { NULL, 5 },  { NULL, 6 },  { NULL, 6 },  { NULL, 1 },  { NULL, 1 },  { NULL, 4 },  { NULL, 3 },  { NULL, 2 }, 
 { NULL, 5 },  { NULL, 3 },  { NULL, 3 },  { NULL, 2 },  { NULL, 2 },  { NULL, 4 },  { NULL, 4 },  { NULL, 3 }, 
 { NULL, 1 },  { NULL, 5 },  { NULL, 1 },  { NULL, 1 },  { NULL, 2 },  { NULL, 4 },  { NULL, 2 },  { NULL, 1 }, 
 { NULL, 1 },  { NULL, 4 },  { NULL, 1 },  { NULL, 2 },  { NULL, 6 },  { NULL, 2 },  { NULL, 2 },  { NULL, 0 }, 
 { NULL, 0 },  { NULL, 0 },  { NULL, 6 },  { NULL, 4 },  { NULL, 1 },  { NULL, 1 },  { NULL, 1 },  { NULL, 2 }, 
 { NULL, 2 },  { NULL, 5 },  { NULL, 4 },  { NULL, 3 },  { NULL, 1 },  { NULL, 7 },  { NULL, 2 },  { NULL, 2 }, 
 { NULL, 4 },  { NULL, 4 },  { NULL, 5 },  { NULL, 1 },  { NULL, 1 } };

VOID InitSyscallData(IN QWORD syscallId, IN BYTE hookInstructionOffset, IN BYTE hookedInstructionLength,
    IN SYSCALL_HANDLER handler, IN BOOL hookReturn, IN SYSCALL_HANDLER returnHandler)
{
    syscallsData[syscallId].hookInstructionOffset = hookInstructionOffset;
    syscallsData[syscallId].hookedInstructionLength = hookedInstructionLength;
    syscallsData[syscallId].hookReturnEvent = hookReturn;
    syscallsData[syscallId].handler = handler;
    syscallsData[syscallId].returnHandler = returnHandler;
}

VOID GetParameters(OUT QWORD_PTR params, IN BYTE count)
{
    PREGISTERS regs = &GetVMMStruct()->guestRegisters;
    QWORD paramsStart = regs->rsp + 4 * sizeof(QWORD);
    switch(count)
    {
        case 17:
        case 16:
        case 15:
        case 14:
        case 13:
        case 12:
        case 11:
        case 10:
        case 9:
        case 8:
        case 7:
        case 6:
        case 5:
            CopyGuestMemory(params + 4, paramsStart, (count - 4) * sizeof(QWORD));
        case 4:
            params[3] = regs->r9;
        case 3:
            params[2] = regs->r8;
        case 2:
            params[1] = regs->rdx;
        case 1:
            params[0] = regs->rcx;
    }
}

STATUS HandleNtOpenPrcoess()
{
    PCURRENT_GUEST_STATE state = GetVMMStruct();
    PSHARED_CPU_DATA shared = state->currentCPU->sharedData;
    PREGISTERS regs = &state->guestRegisters;
    QWORD params[4], pid;
    GetParameters(params, 4);
    CopyGuestMemory(&pid, params[3], sizeof(QWORD));
    Print("PID: %8\n", pid);
    // Emulate replaced instruction: sub rsp,38h
    regs->rsp -= 0x38;
    regs->rip += syscallsData[NT_OPEN_PROCESS].hookedInstructionLength;
    // End emulation
    return STATUS_SUCCESS;
}

STATUS HandleNtCreateUserProcess()
{
    PCURRENT_GUEST_STATE state = GetVMMStruct();
    PSHARED_CPU_DATA shared = state->currentCPU->sharedData;
    PREGISTERS regs = &state->guestRegisters;
    QWORD guestStack[50];
    // Emulate replaced instruction: push rbp
    ASSERT(CopyMemoryToGuest(regs->rsp - 8, &regs->rbp, sizeof(QWORD)) == STATUS_SUCCESS);
    regs->rsp -= 8;
    regs->rip += syscallsData[NT_CREATE_USER_PROCESS].hookedInstructionLength;
    // End emulation
    return STATUS_SUCCESS;
}

STATUS HandleNtOpenPrcoessReturn()
{

}