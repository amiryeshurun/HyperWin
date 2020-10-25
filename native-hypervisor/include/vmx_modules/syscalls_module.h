#ifndef __SYSCALLS_MODULE_H_
#define __SYSCALLS_MODULE_H_

#include <types.h>
#include <vmm/vmm.h>
#include <vmx_modules/module.h>

typedef struct _SYSCALLS_MODULE_EXTENSION
{
    PMODULE kppModule;
    BYTE_PTR ntoskrnl;
    BYTE_PTR win32k;
} SYSCALLS_MODULE_EXTENSION, *PSYSCALLS_MODULE_EXTENSION;

STATUS SyscallsModuleInitialize(IN PSHARED_CPU_DATA sharedData, IN PMODULE module, IN PGENERIC_MODULE_DATA initData);
STATUS SyscallsHandleMsrWrite(IN PCURRENT_GUEST_STATE data);
STATUS LocateSSDT(IN BYTE_PTR lstar, OUT BYTE_PTR ssdt);

#endif