#ifndef __GUEST_COMMUNICATION_H_
#define __GUEST_COMMUNICATION_H_

#include <error_codes.h>
#include <types.h>
#include <win_kernel/file.h>

// Module name
#define COMMUNICATION_MODULE_NAME "HyperWin Communication Module"

// Used to determine if a vm-call was executed in order to read data
// from the communication block
#define VMCALL_COMMUNICATION_BLOCK 0x487970657257696e

// Communication block different operations
typedef QWORD OPERATION, *POPERATION;

#define OPERATION_INIT                   0x4857494e4954    // HWINIT ASCII
#define OPERATION_PROTECTED_PROCESS      0x70726f74656374  // PROTECT ASCII
#define OPERATION_COMPLETED              0x444f4e45        // DONE ASCII
#define OPERATION_PROTECT_FILE_DATA      0x46494c455054    // FILEPT ASCII
#define OPERATION_REMOVE_FILE_PROTECTION 0x52454d4f564546  // REMOVEF ASCII
#define OPERATION_CREATE_NEW_GROUP       0x4e455747524f5550 // NEWGROUP ASCII

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
            DWORD encodingType;
        } protectFileData;
        struct _REMOVE_PROTECTED_FILE
        {
            HANDLE fileHandle;
        } removeProtectedFile;
        struct _CREATE_NEW_GROUP
        {
            QWORD groupId;
            BOOL includeSelf;
        } createNewGroup;
    } argumentsUnion;
} GENERIC_COM_STRUCT, *PGENERIC_COM_STRUCT;

typedef struct _COMMUNICATION_PIPE
{
    QWORD physicalAddress;
    BYTE_PTR virtualAddress;
    QWORD currentOffset;
} COMMUNICATION_PIPE, *PCOMMUNICATION_PIPE;

typedef STATUS (*COMMUNICATION_FUNCTION)(IN PGENERIC_COM_STRUCT);

typedef struct _COMMUNICATION_MODULE_EXTENSION
{
    QWORD_MAP operationToFunction;
    COMMUNICATION_PIPE readPipe;
    COMMUNICATION_PIPE writePipe;
} COMMUNICATION_MODULE_EXTENSION, *PCOMMUNICATION_MODULE_EXTENSION;

// Init
STATUS ComModuleInitializeAllCores(IN PMODULE module);
STATUS ComModuleInitializeSingleCore(IN PMODULE module);

// VM-Exit handlers
STATUS ComHandleVmCall(IN PCURRENT_GUEST_STATE data, IN PMODULE module);
STATUS ComHandleCpuid(IN PCURRENT_GUEST_STATE data, PMODULE module);

// Utils
VOID ComInitPipe(OUT PCOMMUNICATION_PIPE pipe, IN QWORD physicalAddress, IN BYTE_PTR virtualAddress,
    IN QWORD currentOffset);
STATUS ComValidateCaller();
STATUS ComParseCommunicationBlock(IN BYTE_PTR comminucationBlockAddress,
    IN QWORD offsetWithinPipe, OUT POPERATION operation, OUT PGENERIC_COM_STRUCT* arguments);

// Operations
STATUS ComHandleInit(IN PGENERIC_COM_STRUCT args);
STATUS ComHandleProtectProcess(IN PGENERIC_COM_STRUCT args);
STATUS ComHandleHideFileData(IN PGENERIC_COM_STRUCT args);
STATUS ComHandleRemoveProtectedFile(IN PGENERIC_COM_STRUCT args);
STATUS ComHandleCreateNewGroup(IN PGENERIC_COM_STRUCT args);

#endif