#ifndef __KERNEL_OBJECTS_H_
#define __KERNEL_OBJECTS_H_

#include <types.h>
#include <error_codes.h>

#define KPRC 0x0
#define KPRCB 0x1
#define OBJECT_HEADER 0x2
#define ETHREAD 0x3
#define EPROCESS 0x4
#define FILE_OBJECT 0x5
 
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
    ETHREAD_THREAD_ID = 0x480
};

enum
{
    EPROCESS_PID = 0x440,
    EPROCESS_OBJECT_TABLE = 0x570,
    EPROCESS_SIGN_LEVEL = 0x878,
    EPROCESS_SECTION_SIGN_LEVEL = 0x879,
    EPROCESS_PROTECTION = 0x87a,
    EPROCESS_EXE_NAME = 0x5a8
};

enum
{
    FILE_OBJECT_TYPE = 0x0,
    FILE_OBJECT_FILE_NAME = 0x58,
    FILE_OBJECT_SCB = 0x18
};

STATUS ObjGetCurrent_ETHREAD(OUT BYTE_PTR* ethread);
STATUS ObjGetCurrent_EPROCESS(OUT BYTE_PTR* eprocess);
STATUS ObjGet_ETHREAD_field(IN QWORD object, IN QWORD field, OUT PVOID value);
STATUS ObjGet_EPROCESS_field(IN QWORD object, IN QWORD field, OUT PVOID value);
STATUS ObjGet_FILE_OBJECT_field(IN QWORD object, IN QWORD field, OUT PVOID value);
STATUS ObjGetObjectField(IN BYTE objectType, IN QWORD object, IN QWORD field, OUT PVOID value);
STATUS ObjTranslateHandleToObject(IN HANDLE handle, IN BYTE_PTR handleTable, OUT BYTE_PTR* object);

typedef struct _WIN_KERNEL_UNICODE_STRING
{
    WORD length;
    WORD maxLength;
    QWORD address;
} WIN_KERNEL_UNICODE_STRING, *PWIN_KERNEL_UNICODE_STRING;

#endif