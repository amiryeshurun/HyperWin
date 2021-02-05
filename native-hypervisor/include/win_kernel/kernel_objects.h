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
#define DEVICE_OBJECT 0x6
#define DRIVER_OBJECT 0x7
#define VPB 0x8
#define IRP 0x9
#define IO_STACK_LOCATION 0xa
#define MDL 0xb
#define IO_STATUS_BLOCK 0xc
#define FAST_IO_DISPATCH 0xd

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
    FILE_OBJECT_SCB = 0x18,
    FILE_OBJECT_VPB = 0x10
};

enum
{
    DEVICE_OBJECT_DRIVER_OBJECT = 0x8,
    DEVICE_OBJECT_FLAGS = 0x30
};

enum
{
    DRIVER_OBJECT_NAME = 0x38,
    DRIVER_OBJECT_FAST_IO_DISPATCH = 0x50,
    DRIVER_OBJECT_MAJOR_FUNCTION = 0x70
};

enum
{
    VPB_DEVICE_OBJECT = 0x8
};

enum
{
    IRP_MDL = 0x8,
    IRP_FLAGS = 0x10,
    IRP_SYSTEM_BUFFER = 0x18,
    IRP_IO_STATUS = 0x30,
    IRP_USER_IOSB = 0x48,
    IRP_USER_BUFFER = 0x70,
    IRP_TAIL_IO_STACK_LOCATION = 0xb8
};

enum
{
    IO_STACK_LOCATION_FILE_OBJECT = 0x30
};

enum
{
    MDL_SYSTEM_VA = 0x18
};

enum
{
    IO_STATUS_BLOCK_INFORMATION = 0x8
};

enum
{
    FAST_IO_DISPATCH_FAST_IO_READ = 0x10
};

STATUS ObjGetCurrent_ETHREAD(OUT BYTE_PTR* ethread);
STATUS ObjGetCurrent_EPROCESS(OUT BYTE_PTR* eprocess);
STATUS ObjGetObjectField(IN BYTE objectType, IN QWORD object, IN QWORD field, OUT PVOID value);
STATUS ObjTranslateHandleToObject(IN HANDLE handle, IN BYTE_PTR handleTable, OUT BYTE_PTR* object);

typedef struct _WIN_KERNEL_UNICODE_STRING
{
    WORD length;
    WORD maxLength;
    DWORD dummy;
    QWORD address;
} WIN_KERNEL_UNICODE_STRING, *PWIN_KERNEL_UNICODE_STRING;

#endif