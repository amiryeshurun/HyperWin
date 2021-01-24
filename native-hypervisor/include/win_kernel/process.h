#ifndef __WIN_KERNEL_PROCESS_H_
#define __WIN_KERNEL_PROCESS_H_

#include <types.h>
#include <error_codes.h>
#include <utils/set.h>

enum
{
    PS_PROTECTED_SYSTEM = 0x72,
    PS_PROTECTED_WINTCB = 0x62,
    PS_PROTECTED_WINTCB_LIGHT = 0x61,
    PS_PROTECTED_NONE = 0x0
};

enum
{
    PS_PROTECTED_SIGNER_WINTCB = 0x6
};

typedef struct _PROCESS_GROUP
{
    QWORD groupId;
    QWORD_SET processes;
} PROCESS_GROUP, *PPROCESS_GROUP;

VOID PspInit();
STATUS PspMarkProcessProtected(IN QWORD eprocess, IN BYTE protectionLevel, IN BYTE signLevel, 
    IN BYTE sectionSignLevel);
STATUS PspCreateNewGroup(IN QWORD groupId, IN BOOL includeSelf);
QWORD PspGetCurrentThreadId();

#endif