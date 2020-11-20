#ifndef __KERNEL_OBJECTS_H_
#define __KERNEL_OBJECTS_H_

#include <types.h>
#include <error_codes.h>

#define KPRC 0x0
#define KPRCB 0x1
#define OBJECT_HEADER 0x2
#define ETHREAD 0x3
#define EPROCESS 0x4
 
enum
{
    KPRC_KPRCB = 0x180
};

enum
{
    KPRCB_KTHREAD = 0x8
};

enum
{
    OBJECT_HEADER_BODY = 0x30
};

enum
{
    ETHREAD_KAPC_STATE = 0x98,
    ETHREAD_KPROCESS = 0x220,
};

enum
{
    EPROCESS_PID = 0x440,
    EPROCESS_OBJECT_TABLE = 0x570,
    EPROCESS_SIGN_LEVEL = 0x878,
    EPROCESS_SECTION_SIGN_LEVEL = 0x879,
    EPROCESS_PROTECTION = 0x87a
};

STATUS GetCurrent_ETHREAD(OUT BYTE_PTR* ethread);
STATUS GetCurrent_EPROCESS(OUT BYTE_PTR* eprocess);
STATUS Get_ETHREAD_field(IN IN BYTE_PTR object, IN QWORD field, OUT PVOID value);
STATUS Get_EPROCESS_field(IN BYTE_PTR object, IN QWORD field, OUT PVOID value);
STATUS GetObjectField(IN BYTE objectType, IN BYTE_PTR object, IN QWORD field, OUT PVOID value);
STATUS TranslateHandleToObject(IN HANDLE handle, IN BYTE_PTR handleTable, OUT BYTE_PTR* object);

#endif