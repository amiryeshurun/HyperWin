#ifndef __COMMUNICATION_STRUCTS_H_
#define __COMMUNICATION_STRUCTS_H_

#include <guest_communication/communication_operations.h>

#define FILE_PATH_MAX_LENGTH 256

typedef struct _GENERIC_COM_STRUCT
{
    OPERATION operation;
    union 
    {
        struct _INIT_ARGS
        {
            BOOL isMessageAvailable;
            QWORD messageLength;
            BYTE message[PAGE_SIZE];
        } initArgs;
        struct _PROTECT_PROCESS
        {
            HANDLE handle;
        } protectProcess;
        struct _CLEANUP
        {
            union
            {
                QWORD readOffset;
                QWORD status;
            };
            QWORD readLength;
        } cleanup;
        struct _PROTECT_FILE_DATA
        {
            HANDLE fileHandle;
            QWORD contentLength;
            BYTE content[FILE_PATH_MAX_LENGTH];
            DWORD protectionOperation;
        } protectFileData;
    } argumentsUnion;
} GENERIC_COM_STRUCT, *PGENERIC_COM_STRUCT;

typedef struct _COMMUNICATION_PIPE
{
    QWORD physicalAddress;
    BYTE_PTR virtualAddress;
    QWORD currentOffset;
} COMMUNICATION_PIPE, *PCOMMUNICATION_PIPE;

#endif