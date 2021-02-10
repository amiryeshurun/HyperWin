#include <win_kernel/process.h>
#include <vmm/memory_manager.h>
#include <debug.h>
#include <win_kernel/syscall_handlers.h>
#include <utils/map.h>
#include <vmm/vmm.h>
#include <vmx_modules/guest_communication_module.h>
#include <vmm/exit_reasons.h>
#include <utils/utils.h>
#include <win_kernel/kernel_objects.h>
#include <vmm/vmcs.h>
#include <vmm/vm_operations.h>

STATUS ComModuleInitializeAllCores(IN PMODULE module)
{
    PCOMMUNICATION_MODULE_EXTENSION extension;
    PSHARED_CPU_DATA sharedData;
    QWORD physicalReadPipe, physicalWritePipe;

    sharedData = VmmGetVmmStruct()->currentCPU->sharedData;
    PrintDebugLevelDebug("Starting initialization of communication module for all cores\n");
    // Init module & name
    MdlInitModule(module);
    MdlSetModuleName(module, COMMUNICATION_MODULE_NAME);
    // Register VM-Exit handlers
    MdlRegisterVmExitHandler(module, EXIT_REASON_VMCALL, ComHandleVmCall);
    MdlRegisterVmExitHandler(module, EXIT_REASON_CPUID, ComHandleCpuid);
    MdlRegisterModule(module);
    // Allocate space for module extension
    sharedData->heap.allocate(&sharedData->heap, sizeof(COMMUNICATION_MODULE_EXTENSION), &module->moduleExtension);
    // Init extension
    HwSetMemory(module->moduleExtension, 0, sizeof(COMMUNICATION_MODULE_EXTENSION));
    extension = module->moduleExtension;
    MapCreate(&extension->operationToFunction, BasicHashFunction, BASIC_HASH_LEN, DefaultEqualityFunction);
    // Register all operation handlers
    MapSet(&extension->operationToFunction, OPERATION_INIT, ComHandleInit);
    MapSet(&extension->operationToFunction, OPERATION_PROTECTED_PROCESS, ComHandleProtectProcess);
    MapSet(&extension->operationToFunction, OPERATION_PROTECT_FILE_DATA, ComHandleHideFileData);
    MapSet(&extension->operationToFunction, OPERATION_REMOVE_FILE_PROTECTION, ComHandleRemoveProtectedFile);
    MapSet(&extension->operationToFunction, OPERATION_CREATE_NEW_GROUP, ComHandleCreateNewGroup);
    // Allocate space for read & write pipes
    if(BiosAllocateMemoryUsingMemoryMap(sharedData->allRam, sharedData->memoryRangesCount, LARGE_PAGE_SIZE, &physicalReadPipe))
    {
        Print("Allocation of %8 bytes failed.\n", LARGE_PAGE_SIZE);
        ASSERT(FALSE);
    }
    if(BiosAllocateMemoryUsingMemoryMap(sharedData->allRam, sharedData->memoryRangesCount, LARGE_PAGE_SIZE, &physicalWritePipe))
    {
        Print("Allocation of %8 bytes failed.\n", LARGE_PAGE_SIZE);
        ASSERT(FALSE);
    }
    // Save communication block area in global memory & initialize
    HwSetMemory(PhysicalToVirtual(physicalReadPipe), 0, LARGE_PAGE_SIZE);
    ComInitPipe(&extension->readPipe, physicalReadPipe, PhysicalToVirtual(physicalReadPipe), 0);
    HwSetMemory(PhysicalToVirtual(physicalWritePipe), 0 , LARGE_PAGE_SIZE);
    ComInitPipe(&extension->writePipe, physicalWritePipe, PhysicalToVirtual(physicalWritePipe), 0);
    PrintDebugLevelDebug("Shared cores data successfully initialized for communication module\n");

    return STATUS_SUCCESS;
}

STATUS ComModuleInitializeSingleCore(IN PMODULE module)
{
    PSINGLE_CPU_DATA data;

    data = VmmGetVmmStruct()->currentCPU;
    PrintDebugLevelDebug("Communication initialization for core #%d\n", data->coreIdentifier);
    // Reserved for future usage
    PrintDebugLevelDebug("Communication initialization for core #%d - DONE\n", data->coreIdentifier);

    return STATUS_SUCCESS;
}


STATUS ComHandleVmCall(IN PCURRENT_GUEST_STATE data, PMODULE module)
{
    PREGISTERS regs;
    OPERATION operation;
    PGENERIC_COM_STRUCT args;
    QWORD offsetWithinPipe;
    COMMUNICATION_FUNCTION function;
    PCOMMUNICATION_MODULE_EXTENSION extension;
    STATUS status = STATUS_SUCCESS;

    regs = &data->guestRegisters;
    // Should we handle the call?
    if(regs->rax != VMCALL_COMMUNICATION_BLOCK)
        return STATUS_VM_EXIT_NOT_HANDLED;
    // Get offset within the read pipe
    offsetWithinPipe = regs->rbx;
    // Load module extension
    extension = module->moduleExtension;
    // Increament rip (vmcall opcode)
    regs->rip += vmread(VM_EXIT_INSTRUCTION_LEN);
    // Check who executed the vmcall instruction
    if((status = ComValidateCaller()) != STATUS_SUCCESS)
    {
        Print("Could not validate the VMCALL caller: %d\n", status);
        goto cleanup;
    }
    // Get the data from the communication block (hypervisor read pipe)
    SUCCESS_OR_CLEANUP(ComParseCommunicationBlock(extension->readPipe.virtualAddress, offsetWithinPipe,
                        &operation, &args));
    // Handle the vmcall according to the operation code
    if((function = MapGet(&extension->operationToFunction, operation)) == MAP_KEY_NOT_FOUND)
    {
        Print("Unsupported operation: %8\n", operation);
        status = STATUS_UNKNOWN_COMMUNICATION_OPERATION;
        goto cleanup;
    }
    // If a handler was found, handle the call
    status = function(args);

cleanup:
    regs->rax = status;
    return status;
}

STATUS ComHandleCpuid(IN PCURRENT_GUEST_STATE data, PMODULE module)
{
    QWORD leaf, physicalCommunication;
    PREGISTERS regs;
    PCOMMUNICATION_MODULE_EXTENSION extension;
    STATUS status = STATUS_VM_EXIT_NOT_HANDLED;

    regs = &data->guestRegisters;
    leaf = regs->rax;
    extension = module->moduleExtension;
    // Guest asked for physical address of read pipe
    if(leaf == CPUID_GET_READ_PIPE)
    {
        physicalCommunication = extension->readPipe.physicalAddress;
        Print("Received a request for read pipe base address: %8\n", physicalCommunication);
        regs->rdx = physicalCommunication >> 32;
        regs->rax = physicalCommunication & 0xffffffffULL;
        status = STATUS_SUCCESS;
        goto cleanup;
    }
    // Guest asked for physical address of write pipe
    else if(leaf == CPUID_GET_WRITE_PIPE)
    {
        physicalCommunication = extension->writePipe.physicalAddress;
        Print("Received a request for write pipe base address: %8\n", physicalCommunication);
        regs->rdx = physicalCommunication >> 32;
        regs->rax = physicalCommunication & 0xffffffffULL;
        status = STATUS_SUCCESS;
        goto cleanup;
    }

cleanup:
    if(status == STATUS_SUCCESS)
        regs->rip += vmread(VM_EXIT_INSTRUCTION_LEN);
    return status;
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
    STATUS status = STATUS_SUCCESS;

    SUCCESS_OR_CLEANUP(PspMarkProcessProtected(args->argumentsUnion.protectProcess.handle,
         PS_PROTECTED_WINTCB_LIGHT, 0x3e, 0xc));
    Print("Successfully marked process as protected\n");

cleanup:
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
    else
        Print("Successfully created a new group: %d\n", args->argumentsUnion.createNewGroup.groupId);
    
    args->argumentsUnion.cleanup.status = OPERATION_COMPLETED;
    return status;
}

REGISTER_MODULE(ComModuleInitializeAllCores, ComModuleInitializeSingleCore, communication);