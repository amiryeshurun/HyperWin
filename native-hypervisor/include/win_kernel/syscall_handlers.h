#ifndef __SYSCALL_HANDLERS_H_
#define __SYSCALL_HANDLERS_H_

#include <types.h>
#include <vmm/vmm.h>
#include <win_kernel/file.h>

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
    BYTE params;
    SYSCALL_HANDLER returnHandler;
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
    QWORD returnAddress;
    union
    {
        struct
        {
            PHIDDEN_FILE_RULE rule;
            QWORD ioStatusBlock;
            QWORD userBuffer;
        } NtReadFile;
    } dataUnion;
} SYSCALL_EVENT, *PSYSCALL_EVENT;

VOID ShdInitSyscallData(IN QWORD syscallId, IN BYTE hookInstructionOffset, IN BYTE hookedInstructionLength,
    IN SYSCALL_HANDLER handler, IN BOOL hookReturn, IN SYSCALL_HANDLER returnHandler);
VOID ShdHookReturnEvent(IN QWORD syscallId, IN QWORD rsp, IN QWORD threadId);
STATUS ShdHandleNtOpenPrcoess();
STATUS ShdHandleNtCreateUserProcess();
STATUS ShdHandleNtOpenPrcoessReturn();
STATUS ShdHandleNtReadFile();
STATUS ShdHandleNtReadFileReturn();

extern QWORD __ntDataStart;

#endif
