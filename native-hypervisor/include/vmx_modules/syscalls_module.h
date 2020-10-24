#ifndef __SYSCALLS_MODULE_H_
#define __SYSCALLS_MODULE_H_

#include <types.h>
#include <vmm/vmm.h>
#include <vmx_modules/module.h>

typedef struct _SYSCALLS_MODULE_EXTENSION
{
    BYTE_PTR ntoskrnl;
    BYTE_PTR win32k;
} SYSCALLS_MODULE_EXTENSION, *PSYSCALLS_MODULE_EXTENSION;

STATUS SyscallsModuleInitialize(IN PSHARED_CPU_DATA sharedData, IN PMODULE module);
STATUS SyscallsHandleMsrWrite(IN PCURRENT_GUEST_STATE data);

#endif