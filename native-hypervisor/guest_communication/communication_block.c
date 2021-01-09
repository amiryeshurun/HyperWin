#include <guest_communication/communication_block.h>
#include <win_kernel/process.h>
#include <guest_communication/communication_structs.h>
#include <guest_communication/communication_operations.h>
#include <vmm/memory_manager.h>
#include <debug.h>
#include <win_kernel/kernel_objects.h>
#include <win_kernel/syscall_handlers.h>
#include <utils/map.h>
#include <vmm/vmm.h>

QWORD_MAP operationToFunction;

VOID ComInit()
{
    Print("Initializing communication structures\n");
    MapCreate(&operationToFunction, BasicHashFunction, BASIC_HASH_LEN, DefaultEqualityFunction);
    MapSet(&operationToFunction, OPERATION_INIT, ComInit);
    MapSet(&operationToFunction, OPERATION_PROTECTED_PROCESS, ComHandleProtectProcess);
    MapSet(&operationToFunction, OPERATION_PROTECT_FILE_DATA, ComHandleHideFileData);
    MapSet(&operationToFunction, OPERATION_REMOVE_FILE_PROTECTION, ComHandleRemoveProtectedFile);
    MapSet(&operationToFunction, OPERATION_CREATE_NEW_GROUP, ComHandleCreateNewGroup);
    Print("Done initializing communication structures\n");
}

VOID ComHandleVmCallCommunication(IN PCURRENT_GUEST_STATE data)
{
    PREGISTERS regs;
    OPERATION operation;
    PGENERIC_COM_STRUCT args;
    QWORD offsetWithinPipe;
    COMMUNICATION_FUNCTION function;
    STATUS status;

    regs = &data->guestRegisters;
    offsetWithinPipe = regs->rbx;
    
    if((status = ComValidateCaller()) != STATUS_SUCCESS)
    {
        Print("Could not validate the VMCALL caller: %d\n", status);
        goto HandleVmCallCleanup;
    }

    if((status = ComParseCommunicationBlock(data->currentCPU->sharedData->readPipe.virtualAddress, offsetWithinPipe,
        &operation, &args)) != STATUS_SUCCESS)
        goto HandleVmCallCleanup;

    if((function = MapGet(&operationToFunction, operation)) == MAP_KEY_NOT_FOUND)
    {
        Print("Unsupported operation: %8\n", operation);
        status = STATUS_UNKNOWN_COMMUNICATION_OPERATION;
        goto HandleVmCallCleanup;
    }
    // If a handler was found, handle the call
    status = function(args);

HandleVmCallCleanup:
    regs->rax = status;
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

STATUS ComHandleInit(IN PGENERIC_COM_STRUCT args)
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

STATUS ComHandleProtectProcess(IN PGENERIC_COM_STRUCT args)
{
    STATUS status;

    if((status = PspMarkProcessProtected(args->argumentsUnion.protectProcess.handle,
         PS_PROTECTED_WINTCB_LIGHT, 0x3e, 0xc)) != STATUS_SUCCESS)
        return status;
    Print("Successfully marked process as protected\n");

    args->argumentsUnion.cleanup.status = OPERATION_COMPLETED;
    return status;
}

STATUS ComHandleHideFileData(IN PGENERIC_COM_STRUCT args)
{
    HANDLE fileHandle;
    BYTE_PTR content;
    STATUS status;

    fileHandle = args->argumentsUnion.protectFileData.fileHandle;
    content = args->argumentsUnion.protectFileData.content;
    if((status = FileAddNewProtectedFile(fileHandle, content,
        args->argumentsUnion.protectFileData.contentLength,
        args->argumentsUnion.protectFileData.encodingType)) == STATUS_SUCCESS)
        Print("The content of the file will be hidden from now on\n");

    args->argumentsUnion.cleanup.status = OPERATION_COMPLETED;
    return status;
}

STATUS ComHandleRemoveProtectedFile(IN PGENERIC_COM_STRUCT args)
{
    STATUS status;
    
    if((status = FileRemoveProtectedFile(args->argumentsUnion.removeProtectedFile.fileHandle)) !=
        STATUS_SUCCESS)
        Print("Unable to remove protection from file\n");

    args->argumentsUnion.cleanup.status = OPERATION_COMPLETED;
    return status;
}

STATUS ComHandleCreateNewGroup(IN PGENERIC_COM_STRUCT args)
{
    STATUS status;

    if((status = PspCreateNewGroup(args->argumentsUnion.createNewGroup.groupId, 
        args->argumentsUnion.createNewGroup.includeSelf)) != STATUS_SUCCESS)
        Print("Could not create a new group\n");
    
    args->argumentsUnion.cleanup.status = OPERATION_COMPLETED;
    return status;
}