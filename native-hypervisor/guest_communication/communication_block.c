#include <guest_communication/communication_block.h>
#include <win_kernel/process.h>
#include <guest_communication/communication_structs.h>
#include <guest_communication/communication_operations.h>
#include <vmm/memory_manager.h>
#include <debug.h>
#include <win_kernel/kernel_objects.h>
#include <win_kernel/syscall_handlers.h>

STATUS ComHandleVmCallCommunication(IN PCURRENT_GUEST_STATE data)
{
    PREGISTERS regs;
    OPERATION operation;
    PGENERIC_COM_STRUCT args;
    QWORD offsetWithinPipe;
    STATUS status;

    regs = &data->guestRegisters;
    offsetWithinPipe = regs->rbx;
    
    if((status = ComValidateCaller()) != STATUS_SUCCESS)
    {
        Print("Could not validate the VMCALL caller: %d\n", status);
        return status;
    }

    if(ComParseCommunicationBlock(data->currentCPU->sharedData->readPipe.virtualAddress, offsetWithinPipe,
        &operation, &args))
        return STATUS_COMMUNICATION_PARSING_FAILED;

    switch(operation)
    {
        case OPERATION_INIT:
        {
            regs->rax = ComHandleCommunicationInit(args);
            break;
        }
        case OPERATION_PROTECTED_PROCESS:
        {
            regs->rax = ComHandleCommunicationProtect(args);
            break;
        }
        case OPERATION_PROTECT_FILE_DATA:
        {
            regs->rax = ComHandleCommunicationHideData(args);
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


VOID ComInitPipe(OUT PCOMMUNICATION_PIPE pipe, IN QWORD physicalAddress, IN BYTE_PTR virtualAddress,
    IN QWORD currentOffset)
{
    pipe->physicalAddress = physicalAddress;
    pipe->virtualAddress = virtualAddress;
    pipe->currentOffset = currentOffset;
}

STATUS ComParseCommunicationBlock(IN BYTE_PTR comminucationBlockAddress, IN QWORD offsetWithinPipe,
     OUT POPERATION operation, OUT PGENERIC_COM_STRUCT* arguments)
{
    *arguments = (comminucationBlockAddress + offsetWithinPipe);
    *operation = (*arguments)->operation;
    return STATUS_SUCCESS;
}

STATUS ComValidateCaller()
{
    QWORD eprocess;
    CHAR name[20], applicationName[] = { 0x48, 0x79, 0x70, 0x65, 0x72, 0x57, 0x69, 0x6E, 0x2D, 0x43, 0x6F, 0x6E,
         0x73, 0x6F, 0x00 };
         
    ObjGetCurrent_EPROCESS(&eprocess);
    ObjGetObjectField(EPROCESS, eprocess, EPROCESS_EXE_NAME, name);
    // Yes, this is a very shitty way to validate caller's identity. Will be improved later...
    if(!HwCompareMemory(name, applicationName, 15))
        return STATUS_SUCCESS;
    return STATUS_UNKNOWN_VMCALL_CALLER;
}

// Handlers

STATUS ComHandleCommunicationInit(IN PGENERIC_COM_STRUCT args)
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

STATUS ComHandleCommunicationProtect(IN PGENERIC_COM_STRUCT args)
{
    QWORD eprocess, handleTable, protectedProcessEprocess;

    ObjGetCurrent_EPROCESS(&eprocess);
    ObjGetObjectField(EPROCESS, eprocess, EPROCESS_OBJECT_TABLE, &handleTable);
    ObjTranslateHandleToObject(args->argumentsUnion.protectProcess.handle, handleTable, &protectedProcessEprocess);
    if(PspMarkProcessProtected(protectedProcessEprocess, PS_PROTECTED_WINTCB_LIGHT, 0x3e, 0xc) != STATUS_SUCCESS)
        return STATUS_PROTECTED_PROCESS_FAILED;
    
    args->argumentsUnion.cleanup.status = OPERATION_COMPLETED;
    return STATUS_SUCCESS;
}

STATUS ComHandleCommunicationHideData(IN PGENERIC_COM_STRUCT args)
{
    HANDLE fileHandle;
    BYTE content;

    fileHandle = args->argumentsUnion.protectFileData.fileHandle;
    content = args->argumentsUnion.protectFileData.content;
    SyscallsAddNewProtectedFile(fileHandle, content, args->argumentsUnion.protectFileData.contentLength);
    Print("The content of the file will be hidden from now on\n");

    args->argumentsUnion.cleanup.status = OPERATION_COMPLETED;
    return STATUS_SUCCESS;
}