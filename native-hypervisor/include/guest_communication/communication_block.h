#ifndef __COMMUNICATION_BLOCK_H_
#define __COMMUNICATION_BLOCK_H_

#include <error_codes.h>
#include <types.h>
#include <vmm/vmm.h>
#include <guest_communication/communication_structs.h>

STATUS ParseCommunicationBlock(IN BYTE_PTR comminucationBlockAddress,
    IN QWORD offsetWithinPipe, OUT POPERATION operation, OUT PGENERIC_COM_STRUCT* arguments);

#endif