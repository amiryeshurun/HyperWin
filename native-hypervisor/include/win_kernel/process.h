#ifndef __WIN_KERNEL_PROCESS_H_
#define __WIN_KERNEL_PROCESS_H_

#include <types.h>
#include <error_codes.h>

enum
{
    PS_PROTECTED_SYSTEM = 0x72,
    PS_PROTECTED_WINTCB = 0x62,
    PS_PROTECTED_NONE = 0x0
};

STATUS MarkProcessProtected(IN QWORD eprocess);

#endif