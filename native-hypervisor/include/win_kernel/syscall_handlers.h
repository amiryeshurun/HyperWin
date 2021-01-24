#ifndef __SYSCALL_HANDLERS_H_
#define __SYSCALL_HANDLERS_H_

#include <types.h>
#include <vmm/vmm.h>
#include <vmx_modules/hooking_module.h>
#include <win_kernel/utils.h>

#define NT_OPEN_PROCESS 0x26
#define NT_CREATE_USER_PROCESS 0xc8
#define NT_READ_FILE 0x6

STATUS ShdHandleNtReadFile(IN PHOOK_CONTEXT context);

extern QWORD __nt_thread_events_segment;

#endif
