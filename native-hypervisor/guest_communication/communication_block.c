#include <guest_communication/communication_block.h>

STATUS ParseCommunicationBlock(IN BYTE_PTR comminucationBlockAddress, IN QWORD offsetWithinPipe,
     OUT POPERATION operation, OUT PGENERIC_COM_STRUCT* arguments)
{
    *arguments = (comminucationBlockAddress + offsetWithinPipe);
    *operation = (*arguments)->operation;
    return STATUS_SUCCESS;
}