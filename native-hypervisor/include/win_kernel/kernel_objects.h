#ifndef __KERNEL_OBJECTS_H_
#define __KERNEL_OBJECTS_H_

#include <types.h>
#include <error_codes.h>

enum
{
    KPRCB_KTHREAD = 0x8,
    ETHREAD_KAPC_STATE = 0x98,
    KAPC_STATE_KPROCESS = 0x20,
    EPROCESS_PID = 0x2E0
};

STATUS GetCurrent_ETHREAD(IN BYTE_PTR kprcb, OUT BYTE_PTR* ethread);
STATUS GetObjectField(IN BYTE_PTR object, IN QWORD field, OUT PVOID value);
STATUS GetCurrent_EPROCESS(IN BYTE_PTR kprcb, OUT BYTE_PTR* eprocess);

#endif