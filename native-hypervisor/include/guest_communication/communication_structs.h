#ifndef __COMMUNICATION_STRUCTS_H_
#define __COMMUNICATION_STRUCTS_H_

#include <guest_communication/communication_operations.h>

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
        struct _CLEANUP
        {
            QWORD readOffset;
            QWORD readLength;
        } cleanup;
    } argumentsUnion;
} GENERIC_COM_STRUCT, *PGENERIC_COM_STRUCT;

#endif