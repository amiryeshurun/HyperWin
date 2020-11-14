#ifndef __KERNEL_OBJECTS_H_
#define __KERNEL_OBJECTS_H_

#include <types.h>
#include <error_codes.h>

enum
{
    KPRCB_KTHREAD = 0x8,
    ETHREAD_KAPC_STATE = 0x98,
    KAPC_STATE_KPROCESS = 0x20,
    ETHREAD_KPROCESS = 0x220,
    KPRC_KPRCB = 0x180,
    EPROCESS_PID = 0x440,
    EPROCESS_OBJECT_TABLE = 0x570,
    OBJECT_HEADER_BODY = 0x30
};

STATUS GetCurrent_ETHREAD(OUT BYTE_PTR* ethread);
STATUS GetObjectField(IN BYTE_PTR object, IN QWORD field, OUT PVOID value);
STATUS GetCurrent_EPROCESS(OUT BYTE_PTR* eprocess);
STATUS TranslateHandleToObject(IN HANDLE handle, IN BYTE_PTR handleTable, OUT BYTE_PTR* object);

#endif