#ifndef __SYSCALL_HANDLERS_H_
#define __SYSCALL_HANDLERS_H_

#include <types.h>
#include <vmm/vmm.h>
#include <vmx_modules/syscalls_module.h>

#define NT_OPEN_PROCESS 0x26

typedef STATUS (*SYSCALL_HANDLER)(IN QWORD_PTR);

STATUS HandleNtOpenPrcoess(IN QWORD_PTR params);

extern QWORD __ntDataStart;

#endif
