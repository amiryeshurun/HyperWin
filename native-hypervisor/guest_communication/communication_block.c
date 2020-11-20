#include <guest_communication/communication_block.h>
#include <win_kernel/process.h>
#include <guest_communication/communication_structs.h>
#include <guest_communication/communication_operations.h>
#include <vmm/memory_manager.h>
#include <debug.h>
#include <win_kernel/kernel_objects.h>

STATUS HandleVmCallCommunication(IN PCURRENT_GUEST_STATE data)
{
    PREGISTERS regs = &(data->guestRegisters);
    OPERATION operation;
    PGENERIC_COM_STRUCT args;
    QWORD offsetWithinPipe = regs->rbx;
    
    if(ParseCommunicationBlock(data->currentCPU->sharedData->readPipe.virtualAddress, offsetWithinPipe,
        &operation, &args))
        return STATUS_COMMUNICATION_PARSING_FAILED;

    switch(operation)
    {
        case OPERATION_INIT:
        {
            regs->rax = HandleCommunicationInit(args);
            break;
        }
        case OPERATION_PROTECTED_PROCESS:
        {
            regs->rax = HandleCommunicationProtect(args);
            break;
        }
        default:
        {
            Print("Unknown operation was sent from guest: %8\n", operation);
            return STATUS_UNKNOWN_COMMUNICATION_OPERATION;
        }
    }

    return STATUS_SUCCESS;
}


VOID InitPipe(OUT PCOMMUNICATION_PIPE pipe, IN QWORD physicalAddress, IN BYTE_PTR virtualAddress,
    IN QWORD currentOffset)
{
    pipe->physicalAddress = physicalAddress;
    pipe->virtualAddress = virtualAddress;
    pipe->currentOffset = currentOffset;
}

STATUS ParseCommunicationBlock(IN BYTE_PTR comminucationBlockAddress, IN QWORD offsetWithinPipe,
     OUT POPERATION operation, OUT PGENERIC_COM_STRUCT* arguments)
{
    *arguments = (comminucationBlockAddress + offsetWithinPipe);
    *operation = (*arguments)->operation;
    return STATUS_SUCCESS;
}

// Handlers

STATUS HandleCommunicationInit(IN PGENERIC_COM_STRUCT args)
{
    if(args->argumentsUnion.initArgs.isMessageAvailable)
    {
        Print("A message from guest: %.b\n", args->argumentsUnion.initArgs.messageLength, 
            args->argumentsUnion.initArgs.message);
    }
    else
        Print("Guest sent INIT without any message\n");
    args->argumentsUnion.cleanup.status = OPERATION_COMPLETED;

    return STATUS_SUCCESS; 
}

STATUS HandleCommunicationProtect(IN PGENERIC_COM_STRUCT args)
{
    QWORD eprocess;
    GetCurrent_EPROCESS(&eprocess);
    if(MarkProcessProtected(eprocess, PS_PROTECTED_WINTCB_LIGHT, 0x3e, 0xc) != STATUS_SUCCESS)
        return STATUS_PROTECTED_PROCESS_FAILED;
    args->argumentsUnion.cleanup.status = OPERATION_COMPLETED;
    return STATUS_SUCCESS;
}