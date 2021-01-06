#ifndef __COMMUNICATION_BLOCK_H_
#define __COMMUNICATION_BLOCK_H_

#include <error_codes.h>
#include <types.h>
#include <vmm/vmm.h>
#include <guest_communication/communication_structs.h>

#define VMCALL_COMMUNICATION_BLOCK 0x487970657257696e

// Utils
VOID ComInitPipe(OUT PCOMMUNICATION_PIPE pipe, IN QWORD physicalAddress, IN BYTE_PTR virtualAddress,
    IN QWORD currentOffset);

STATUS ComValidateCaller();
STATUS ComHandleVmCallCommunication(IN PCURRENT_GUEST_STATE data);
STATUS ComParseCommunicationBlock(IN BYTE_PTR comminucationBlockAddress,
    IN QWORD offsetWithinPipe, OUT POPERATION operation, OUT PGENERIC_COM_STRUCT* arguments);

// Operations
STATUS ComHandleInit(IN PGENERIC_COM_STRUCT args);
STATUS ComHandleProtectProcess(IN PGENERIC_COM_STRUCT args);
STATUS ComHandleHideFileData(IN PGENERIC_COM_STRUCT args);
STATUS ComHandleRemoveProtectedFile(IN PGENERIC_COM_STRUCT args);

#endif