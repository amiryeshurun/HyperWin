#ifndef __WIN_KERNEL_UTILS_
#define __WIN_KERNEL_UTILS_

#include <types.h>
#include <win_kernel/file.h>

typedef struct _THREAD_EVENT
{
    QWORD returnAddress;
    union
    {
        struct
        {
            PHIDDEN_FILE_RULE rule;
            QWORD ioStatusBlock;
            QWORD userBuffer;
        } NtReadFile;
        struct
        {
            PHIDDEN_FILE_RULE rule;
            QWORD bufferAddress;
            QWORD ioStatusBlock;
            BOOL isPhysical;
        } NtfsFsdRead;
        struct
        {
            PHIDDEN_FILE_RULE rule;
            QWORD buffer;
            QWORD ioStatusBlock;
        } NtfsReadCopyA;
    } dataUnion;
} THREAD_EVENT, *PTHREAD_EVENT;

VOID WinGetParameters(OUT QWORD_PTR params, IN BYTE count);
VOID WinHookReturnEvent(IN QWORD rsp, IN QWORD threadId, IN QWORD hookAddress);
PTHREAD_EVENT WinGetEventForThread(IN QWORD threadId);
STATUS FileHandleFastRead(IN PHOOK_CONTEXT context);
STATUS FileHandleFastReadReturn(IN PHOOK_CONTEXT context);
VOID WinInitializeComponents();

#endif