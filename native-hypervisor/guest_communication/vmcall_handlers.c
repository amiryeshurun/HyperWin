#include <guest_communication/vmcall_values.h>
#include <guest_communication/vmcall_handlers.h>
#include <guest_communication/communication_block.h>
#include <guest_communication/communication_structs.h>
#include <guest_communication/communication_operations.h>
#include <vmm/memory_manager.h>
#include <debug.h>

STATUS HandleVmCallCommunication(IN PCURRENT_GUEST_STATE data)
{
    PREGISTERS regs = &(data->guestRegisters);
    OPERATION operation;
    PGENERIC_COM_STRUCT args;
    QWORD offsetWithinPipe = regs->rbx;
    
    if(ParseCommunicationBlock(data->currentCPU->sharedData->readPipe.virtualAddress, offsetWithinPipe, &operation, &args))
        return STATUS_COMMUNICATION_PARSING_FAILED;

    switch(operation)
    {
        case OPERATION_INIT:
        {
            regs->rax = HandleCommunicationInit(args);
            break;
        }
    }

    return STATUS_SUCCESS;
}
