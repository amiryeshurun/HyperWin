#ifndef __SYSCALLS_MODULE_H_
#define __SYSCALLS_MODULE_H_

#include <types.h>
#include <vmm/vmm.h>
#include <vmx_modules/module.h>

#define COUNT_UNTIL_HOOK 1000

typedef struct _SYSCALLS_MODULE_EXTENSION
{
    PMODULE kppModule;
    BOOL startExitCount;
    QWORD exitCount;
    BYTE_PTR ntoskrnl;
    BYTE_PTR win32k;
    BYTE_PTR lstar;
    QWORD guestCr3;
} SYSCALLS_MODULE_EXTENSION, *PSYSCALLS_MODULE_EXTENSION;

STATUS SyscallsModuleInitializeAllCores(IN PSHARED_CPU_DATA sharedData, IN PMODULE module, IN PGENERIC_MODULE_DATA initData);
STATUS SyscallsModuleInitializeSingleCore(IN PSINGLE_CPU_DATA data);
STATUS SyscallsDefaultHandler(IN PCURRENT_GUEST_STATE sharedData, IN PMODULE module);
STATUS SyscallsHandleMsrWrite(IN PCURRENT_GUEST_STATE data, IN PMODULE module);
STATUS LocateSSDT(IN BYTE_PTR lstar, OUT BYTE_PTR* ssdt, IN QWORD guestCr3);
VOID GetSystemTables(IN BYTE_PTR ssdt, OUT BYTE_PTR* ntoskrnl, OUT BYTE_PTR* win32k, IN QWORD guestCr3);
STATUS HookSystemCalls(IN QWORD guestCr3, IN BYTE_PTR ntoskrnl, IN BYTE_PTR win32k, IN QWORD count, ...);

#endif