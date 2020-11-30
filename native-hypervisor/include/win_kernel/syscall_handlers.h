#ifndef __SYSCALL_HANDLERS_H_
#define __SYSCALL_HANDLERS_H_

#include <types.h>
#include <vmm/vmm.h>

#define NT_OPEN_PROCESS 0x26
#define NT_CREATE_USER_PROCESS 0xc8
#define NT_READ_FILE 0x6

typedef STATUS (*SYSCALL_HANDLER)();

// The INT3 instruction used to detect return events of system call is (ADDRESS + 1)
#define CALC_RETURN_HOOK_ADDR(address) (address + 1)

typedef struct _SYSCALL_DATA
{
    // Statically defined
    SYSCALL_HANDLER handler;
    SYSCALL_HANDLER returnHandler;
    BYTE params;
    BYTE hookInstructionOffset;
    BOOL hookReturnEvent;
    QWORD hookedInstructionLength;
    // Dynamically defined
    QWORD hookedInstructionAddress;
    QWORD virtualHookedInstructionAddress;
    BYTE hookedInstrucion[X86_MAX_INSTRUCTION_LEN];
} SYSCALL_DATA, *PSYSCALL_DATA;

typedef struct _SYSCALL_EVENT
{
    QWORD params[17];
} SYSCALL_EVENT, *PSYSCALL_EVENT;

VOID InitSyscallData(IN QWORD syscallId, IN BYTE hookInstructionOffset, IN BYTE hookedInstructionLength,
    IN SYSCALL_HANDLER handler, IN BOOL hookReturn, IN SYSCALL_HANDLER returnHandler);
VOID HookReturnEvent(IN QWORD syscallId, IN QWORD rsp, OUT QWORD_PTR realReturnAddress);
STATUS HandleNtOpenPrcoess();
STATUS HandleNtCreateUserProcess();
STATUS HandleNtOpenPrcoessReturn();

extern QWORD __ntDataStart;

#endif
