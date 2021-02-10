#include <debug.h>
#include <win_kernel/memory_manager.h>
#include <vmm/vmm.h>
#include <win_kernel/kernel_objects.h>
#include <win_kernel/file.h>
#include <win_kernel/process.h>
#include <win_kernel/utils.h>
#include <vmm/memory_manager.h>
#include <win_kernel/component.h>

QWORD_MAP g_filesData;

VOID FileInit()
{
    MapCreate(&g_filesData, BasicHashFunction, BASIC_HASH_LEN, DefaultEqualityFunction);
}

STATUS FileTranslateScbToFcb(IN QWORD scb, OUT QWORD_PTR fcb)
{
    STATUS status = STATUS_SUCCESS;

    SUCCESS_OR_CLEANUP(WinMmCopyGuestMemory(fcb, scb + SCB_FCB_OFFSET, sizeof(QWORD)));

cleanup:
    return status;
}

STATUS FileGetFcbField(IN QWORD fcb, IN QWORD field, OUT PVOID value)
{
    switch(field)
    {
        case FCB_MFT_INDEX:
            return WinMmCopyGuestMemory(value, fcb + field, sizeof(QWORD));
        default:
            Print("Could not find the specified field\n");
            ASSERT(FALSE);
    }
}

STATUS FileAddNewProtectedFile(IN HANDLE fileHandle, IN BYTE_PTR content, IN QWORD contentLength, 
    IN BYTE encodingType)
{
    PSHARED_CPU_DATA shared;
    PHEAP heap;
    PHOOKING_MODULE_EXTENSION ext;
    PHIDDEN_FILE_RULE rule;
    QWORD fileObject, scb, fcb, eprocess, handleTable, fileIndex;
    STATUS status = STATUS_SUCCESS;

    shared = VmmGetVmmStruct()->currentCPU->sharedData;
    heap = &shared->heap;
    // Translate the Handle to an object
    ObjGetCurrent_EPROCESS(&eprocess);
    ObjGetObjectField(EPROCESS, eprocess, EPROCESS_OBJECT_TABLE, &handleTable);
    SUCCESS_OR_CLEANUP(ObjTranslateHandleToObject(fileHandle, handleTable, &fileObject));
    // Get the MFTIndex field
    SUCCESS_OR_CLEANUP(ObjGetObjectField(FILE_OBJECT, fileObject, FILE_OBJECT_SCB, &scb));
    SUCCESS_OR_CLEANUP(FileTranslateScbToFcb(scb, &fcb));
    SUCCESS_OR_CLEANUP(FileGetFcbField(fcb, FCB_MFT_INDEX, &fileIndex));
    // Allocate memory for storing the rule
    heap->allocate(heap, sizeof(HIDDEN_FILE_RULE), &rule);
    heap->allocate(heap, contentLength, &rule->content.data);
    HwCopyMemory(rule->content.data, content, contentLength);
    // Set the rule
    rule->content.length = contentLength;
    rule->rule = FILE_HIDE_CONTENT;
    rule->encoding = encodingType;
    // Get the module extension
    // Map the file to a rule
    MapSet(&g_filesData, fileIndex, rule);
    Print("File rule added for file idx: %8, data: %.b\n", fileIndex, contentLength, content);

cleanup:
    if(status && rule && rule->content.data)
        heap->deallocate(heap, rule->content.data);
    if(status && rule)
        heap->deallocate(heap, rule);
    return status;
}

STATUS FileRemoveProtectedFile(IN HANDLE fileHandle)
{
    PSHARED_CPU_DATA shared;
    PHEAP heap;
    PHOOKING_MODULE_EXTENSION ext;
    PHIDDEN_FILE_RULE rule;
    QWORD eprocess, handleTable, fileObject, scb, fcb, fileIndex;
    STATUS status = STATUS_SUCCESS;

    shared = VmmGetVmmStruct()->currentCPU->sharedData;
    heap = &shared->heap;
    ObjGetCurrent_EPROCESS(&eprocess);
    ObjGetObjectField(EPROCESS, eprocess, EPROCESS_OBJECT_TABLE, &handleTable);
    SUCCESS_OR_CLEANUP(ObjTranslateHandleToObject(fileHandle, handleTable, &fileObject));
    // Get the MFTIndex field
    SUCCESS_OR_CLEANUP(ObjGetObjectField(FILE_OBJECT, fileObject, FILE_OBJECT_SCB, &scb));
    SUCCESS_OR_CLEANUP(FileTranslateScbToFcb(scb, &fcb));
    SUCCESS_OR_CLEANUP(FileGetFcbField(fcb, FCB_MFT_INDEX, &fileIndex));
    // Remove rule from map
    rule = (PHIDDEN_FILE_RULE)MapRemove(&g_filesData, fileIndex);
    if(rule != MAP_KEY_NOT_FOUND)
    {
        // Free the stored data
        heap->deallocate(heap, rule->content.data);
        if(rule->optional.data)
            heap->deallocate(heap, rule->optional.data);
        heap->deallocate(heap, rule);
    }

cleanup:
    return status;
}

STATUS FileGetRuleByIndex(IN QWORD fileIndex, OUT PHIDDEN_FILE_RULE* rule)
{
    return ((*rule = MapGet(&g_filesData, fileIndex)) != MAP_KEY_NOT_FOUND) ?
         STATUS_SUCCESS : STATUS_FILE_NOT_FOUND;
}

VOID FileReplaceDataByRule(IN BYTE_PTR buffer, IN QWORD length, IN PHIDDEN_FILE_RULE rule)
{
    QWORD count, indices[20];
    PWCHAR utf16Ptr;

    if(length >= rule->content.length && 
        (count = MemoryContains(buffer, length, rule->content.data, rule->content.length, indices)) 
            != 0)
    {
        if(rule->encoding == ENCODING_TYPE_UTF_16)
        {
            utf16Ptr = buffer;
            for(QWORD k = 0; k < count; k++)
                for(QWORD i = indices[k] / 2; i < (indices[k] + rule->content.length) / 2; i++)
                    utf16Ptr[i] = L'*';
        }
        else if(rule->encoding == ENCODING_TYPE_UTF_8)
            for(QWORD k = 0; k < count; k++)
                for(QWORD i = indices[k]; i < indices[k] + rule->content.length; i++)
                    buffer[i] = '*';
    }
}

STATUS FileHandleRead(IN PHOOK_CONTEXT context)
{
    PCURRENT_GUEST_STATE currentData;
    PSHARED_CPU_DATA shared;
    PREGISTERS regs;
    QWORD fileObject, ioStackLocation, params[17], fcb, scb, fileIndex, threadId, mdl, ioStatus,
        userBuffer, systemVa;
    PHIDDEN_FILE_RULE hiddenFileRule;
    PTHREAD_EVENT threadEvent;
    STATUS status = STATUS_SUCCESS;

    currentData = VmmGetVmmStruct();
    shared = currentData->currentCPU;
    regs = &currentData->guestRegisters;
    // To support WinGetParameters and WinHookReturnEvent
    regs->rsp += 5 * sizeof(QWORD);
    WinGetParameters(params, context->relatedConfig->params);
    // Get the FileObject for the current file
    ObjGetObjectField(IRP, params[1], IRP_TAIL_IO_STACK_LOCATION, &ioStackLocation);
    ObjGetObjectField(IO_STACK_LOCATION, ioStackLocation, IO_STACK_LOCATION_FILE_OBJECT,
        &fileObject);
    ObjGetObjectField(FILE_OBJECT, fileObject, FILE_OBJECT_SCB, &scb);
    FileTranslateScbToFcb(scb, &fcb);
    FileGetFcbField(fcb, FCB_MFT_INDEX, &fileIndex);
    // Check if the current file is protected
    if(FileGetRuleByIndex(fileIndex, &hiddenFileRule) == STATUS_SUCCESS)
    {
        // The file is indeed protected
        // Get the thread id
        threadId = PspGetCurrentThreadId();
        // Hook return event
        WinHookReturnEvent(regs->rsp, threadId, context->virtualAddress);
        threadEvent = WinGetEventForThread(threadId);
        // Get & Store important information for later
        SUCCESS_OR_CLEANUP(ObjGetObjectField(IRP, params[1], IRP_USER_BUFFER, &userBuffer));
        // User address is used directly
        if(userBuffer)
        {
            threadEvent->dataUnion.NtfsFsdRead.bufferAddress = userBuffer;
            threadEvent->dataUnion.NtfsFsdRead.isPhysical = FALSE;
        }
        else
        {
            // An MDL is being used
            SUCCESS_OR_CLEANUP(ObjGetObjectField(IRP, params[1], IRP_MDL, &mdl));
            SUCCESS_OR_CLEANUP(ObjGetObjectField(MDL, mdl, MDL_SYSTEM_VA, &systemVa));
            if(!systemVa)
            {
                status = STATUS_MDL_ADDRESS_INVALID;
                goto cleanup;
            }
            threadEvent->dataUnion.NtfsFsdRead.bufferAddress = WinMmTranslateGuestPhysicalToPhysicalAddress(systemVa);
            threadEvent->dataUnion.NtfsFsdRead.isPhysical = TRUE;
        }
        // Used to get the readen length
        SUCCESS_OR_CLEANUP(ObjGetObjectField(IRP, params[1], IRP_USER_IOSB, &ioStatus));
        threadEvent->dataUnion.NtfsFsdRead.rule = hiddenFileRule;
        threadEvent->dataUnion.NtfsFsdRead.ioStatusBlock = ioStatus;
    }
cleanup:
    // sub rsp,0A0h
    regs->rsp -= 5 * sizeof(QWORD);
    regs->rsp -= 0xa0;
    regs->rip += context->relatedConfig->instructionLength;
    return status;
}

STATUS FileHandleFastRead(IN PHOOK_CONTEXT context)
{
    PCURRENT_GUEST_STATE currentData;
    PSHARED_CPU_DATA shared;
    PREGISTERS regs;
    QWORD fileObject, ioStackLocation, params[17], fcb, scb, fileIndex, threadId;
    PHIDDEN_FILE_RULE hiddenFileRule;
    PTHREAD_EVENT threadEvent;
    STATUS status = STATUS_SUCCESS;

    currentData = VmmGetVmmStruct();
    shared = currentData->currentCPU;
    regs = &currentData->guestRegisters;
    WinGetParameters(params, context->relatedConfig->params);
    // Get the FileObject for the current file
    fileObject = params[0];
    ObjGetObjectField(FILE_OBJECT, fileObject, FILE_OBJECT_SCB, &scb);
    FileTranslateScbToFcb(scb, &fcb);
    FileGetFcbField(fcb, FCB_MFT_INDEX, &fileIndex);
    // Check if the current file is protected
    if(FileGetRuleByIndex(fileIndex, &hiddenFileRule) == STATUS_SUCCESS)
    {
        // The file is indeed protected
        // Get the thread id
        threadId = PspGetCurrentThreadId();
        // Hook return event
        WinHookReturnEvent(regs->rsp, threadId, context->virtualAddress);
        threadEvent = WinGetEventForThread(threadId);
        // Get & Store important information for later   
        threadEvent->dataUnion.NtfsReadCopyA.rule = hiddenFileRule;     
        threadEvent->dataUnion.NtfsReadCopyA.buffer = params[5];
        // Used to get the readen length
        threadEvent->dataUnion.NtfsReadCopyA.ioStatusBlock = params[6];
    }
cleanup:
    // push rbx
    regs->rsp -= sizeof(QWORD);
    WinMmCopyMemoryToGuest(regs->rsp, &regs->rbx, sizeof(QWORD));
    regs->rip += context->relatedConfig->instructionLength;
    return status;
}

STATUS FileHandleReadReturn(IN PHOOK_CONTEXT context)
{
    PCURRENT_GUEST_STATE state;
    PSHARED_CPU_DATA shared;
    PREGISTERS regs;
    QWORD bufferLength, guestPhysical;
    PHIDDEN_FILE_RULE rule;
    BYTE_PTR hostVirtual;
    PWCHAR utf16Ptr;
    PTHREAD_EVENT threadEvent;
    STATUS status = STATUS_SUCCESS;

    state = VmmGetVmmStruct();
    shared = state->currentCPU->sharedData;
    regs = &state->guestRegisters;
    // Using the thread id, get the saved data
    threadEvent = WinGetEventForThread(PspGetCurrentThreadId());
    // Get the rule found in the hashmap
    rule = threadEvent->dataUnion.NtReadFile.rule;
    // Get the buffer address for the request & its length
    SUCCESS_OR_CLEANUP(ObjGetObjectField(IO_STATUS_BLOCK, threadEvent->dataUnion.NtfsFsdRead.ioStatusBlock,
        IO_STATUS_BLOCK_INFORMATION, &bufferLength));
    // For direct IO only
    if(threadEvent->dataUnion.NtfsFsdRead.isPhysical)
        hostVirtual = PhysicalToVirtual(threadEvent->dataUnion.NtfsFsdRead.bufferAddress);
    // For IO type "neither"
    else
    {
        // Translate to host virtual
        WinMmTranslateGuestVirtualToHostVirtual(threadEvent->dataUnion.NtfsFsdRead.bufferAddress, &hostVirtual);
    }
    // Replace hidden content (if exists)
    FileReplaceDataByRule(hostVirtual, bufferLength, rule);
cleanup:
    // Put back the saved address in the RIP register
    regs->rip = threadEvent->returnAddress;
    return status;
}

STATUS FileHandleFastReadReturn(IN PHOOK_CONTEXT context)
{
    PCURRENT_GUEST_STATE state;
    PSHARED_CPU_DATA shared;
    PREGISTERS regs;
    QWORD bufferLength;
    PHIDDEN_FILE_RULE rule;
    BYTE_PTR hostVirtual;
    PWCHAR utf16Ptr;
    PTHREAD_EVENT threadEvent;
    STATUS status = STATUS_SUCCESS;

    state = VmmGetVmmStruct();
    shared = state->currentCPU->sharedData;
    regs = &state->guestRegisters;
    // Using the thread id, get the saved data
    threadEvent = WinGetEventForThread(PspGetCurrentThreadId());
    // Get the rule found in the hashmap
    rule = threadEvent->dataUnion.NtReadFile.rule;
    // Get the buffer address for the request & its length
    ObjGetObjectField(IO_STATUS_BLOCK, threadEvent->dataUnion.NtfsReadCopyA.ioStatusBlock,
        IO_STATUS_BLOCK_INFORMATION, &bufferLength);
    WinMmTranslateGuestVirtualToHostVirtual(threadEvent->dataUnion.NtfsReadCopyA.buffer, &hostVirtual);
    // Replace hidden content (if exists)
    FileReplaceDataByRule(hostVirtual, bufferLength, rule);
cleanup:
    // Put back the saved address in the RIP register
    regs->rip = threadEvent->returnAddress;
    return status;
}

REGISTER_COMPONENT(FileInit, file);