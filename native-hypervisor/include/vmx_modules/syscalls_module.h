#ifndef __SYSCALLS_MODULE_H_
#define __SYSCALLS_MODULE_H_

#include <types.h>
#include <vmm/vmm.h>
#include <vmx_modules/module.h>
#include <win_kernel/syscall_handlers.h>
#include <utils/map.h>
#include <utils/set.h>
#include <win_kernel/file.h>

// Used to determine how many vm-exits should occur before hooking the system calls
#define COUNT_UNTIL_HOOK 1000

// Used to determine if a physical address is assosiated with a return event or not
#define RETURN_EVENT_FLAG (1 << 30)

typedef struct _SYSCALLS_MODULE_EXTENSION
{
    PSYSCALL_DATA syscallsData;
    BOOL startExitCount;
    QWORD exitCount;
    BYTE_PTR ntoskrnl;
    BYTE_PTR win32k;
    BYTE_PTR lstar;
    QWORD guestCr3;
    QWORD_MAP addressToSyscall;
    QWORD_SET addressSet;
    QWORD_MAP filesData;
} SYSCALLS_MODULE_EXTENSION, *PSYSCALLS_MODULE_EXTENSION;

STATUS SyscallsModuleInitializeAllCores(IN PSHARED_CPU_DATA sharedData, IN PMODULE module, IN PGENERIC_MODULE_DATA initData);
STATUS SyscallsModuleInitializeSingleCore(IN PSINGLE_CPU_DATA data);
STATUS SyscallsDefaultHandler(IN PCURRENT_GUEST_STATE sharedData, IN PMODULE module);
STATUS SyscallsHandleMsrWrite(IN PCURRENT_GUEST_STATE data, IN PMODULE module);
STATUS SyscallsHandleException(IN PCURRENT_GUEST_STATE data, IN PMODULE module);

// Operational functions
STATUS LocateSSDT(IN BYTE_PTR lstar, OUT BYTE_PTR* ssdt, IN QWORD guestCr3);
VOID GetSystemTables(IN BYTE_PTR ssdt, OUT BYTE_PTR* ntoskrnl, OUT BYTE_PTR* win32k, IN QWORD guestCr3);
STATUS HookSystemCalls(IN PMODULE module, IN QWORD guestCr3, IN BYTE_PTR ntoskrnl, IN BYTE_PTR win32k, 
    IN QWORD count, ...);
STATUS AddNewProtectedFile(IN BYTE_PTR path, IN QWORD pathLength, IN BYTE_PTR content, 
    IN QWORD contentLength);

#endif