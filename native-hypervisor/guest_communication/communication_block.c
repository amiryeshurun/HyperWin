#include <guest_communication/communication_block.h>

STATUS ParseCommunicationBlock(IN BYTE_PTR comminucationBlockAddress, OUT POPERATION operation, OUT PGENERIC_COM_STRUCT* arguments)
{
    *arguments = comminucationBlockAddress;
    *operation = (*arguments)->operation;
    return STATUS_SUCCESS;
}