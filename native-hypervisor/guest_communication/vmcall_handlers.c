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

    if(ParseCommunicationBlock(data->currentCPU->sharedData->virtualCommunicationBase, &operation, &args))
        return STATUS_COMMUNICATION_PARSING_FAILED;

    switch(operation)
    {
        case OPERATION_INIT:
        {
            if(args->argumentsUnion.initArgs.isMessageAvailable)
            {
                Print("A message from guest: %.b\n", args->argumentsUnion.initArgs.messageLength, 
                    args->argumentsUnion.initArgs.message);
            }
            else
                Print("Guest initialization without any message\n");
            break;
        }
    }

    return STATUS_SUCCESS;
}
