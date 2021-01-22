#include <win_kernel/syscall_handlers.h>
#include <win_kernel/memory_manager.h>
#include <debug.h>
#include <vmm/vmcs.h>
#include <vmm/vm_operations.h>
#include <win_kernel/kernel_objects.h>
#include <vmx_modules/hooking_module.h>
#include <win_kernel/utils.h>
#include <win_kernel/irp.h>
#include <win_kernel/file.h>

STATUS ShdHandleNtReadFile(IN PHOOK_CONTEXT context)
{
    PCURRENT_GUEST_STATE state;
    PSHARED_CPU_DATA shared;
    PREGISTERS regs;
    QWORD params[17];
    QWORD fileObject, handleTable, eprocess, threadId, ethread, returnAddress, scb, fcb, fileIndex,
        vpb, deviceObj, driverObj, irpMjRead;
    WORD fileType;
    WORD_PTR pfileType = &fileType;
    STATUS status;
    PHIDDEN_FILE_RULE hiddenFileRule;
    WIN_KERNEL_UNICODE_STRING driverName;
    PTHREAD_EVENT threadEvent;

    state = VmmGetVmmStruct();
    shared = state->currentCPU->sharedData;
    regs = &state->guestRegisters;
    // Receive syscall parameters
    WinGetParameters(params, context->relatedConfig->params);
    // Translate the first parameter (file handle) to the corresponding _FILE_OBJECT structure
    ObjGetCurrent_EPROCESS(&eprocess);
    ObjGetObjectField(EPROCESS, eprocess, EPROCESS_OBJECT_TABLE, &handleTable);
    if(ObjTranslateHandleToObject(params[0], handleTable, &fileObject) != STATUS_SUCCESS)
    {
#ifdef DEBUG_HANDLE_TRANSLATION_FAILURE
        Print("Could not translate handle to object, skipping... (Handle value: %8)\n", params[0]);
#endif
        goto NtReadFileEmulateInstruction;
    }
    // Check if this is a file object (See MSDN file object page)
    ObjGetObjectField(FILE_OBJECT, fileObject, FILE_OBJECT_TYPE, &fileType);
    if(fileType != 5)
        goto NtReadFileEmulateInstruction;
    // Try to hook NtfsFsdRead instead of NtReadFile (for kernel mode data protection)
    ObjGetObjectField(FILE_OBJECT, fileObject, FILE_OBJECT_VPB, &vpb);
    if(vpb)
    {
        // Ntfs
        BYTE nameAsBytes[] = { 0x5C, 0x00, 0x46, 0x00, 0x69, 0x00, 0x6C, 0x00, 0x65, 0x00, 0x53, 0x00, 0x79, 0x00, 0x73, 0x00,
         0x74, 0x00, 0x65, 0x00, 0x6D, 0x00, 0x5C, 0x00, 0x4E, 0x00, 0x74, 0x00, 0x66, 0x00, 0x73, 0x00 };
        BYTE driverNameBuffer[50];
        // Check the name of the driver
        ObjGetObjectField(VPB, vpb, VPB_DEVICE_OBJECT, &deviceObj);
        ObjGetObjectField(DEVICE_OBJECT, deviceObj, DEVICE_OBJECT_DRIVER_OBJECT, &driverObj);
        ObjGetObjectField(DRIVER_OBJECT, driverObj, DRIVER_OBJECT_NAME, &driverName);
        WinMmCopyGuestMemory(driverNameBuffer, driverName.address, driverName.length);
        // Is it Ntfs?
        if(!HwCompareMemory(driverNameBuffer, nameAsBytes, 32))
        {
            WinMmCopyGuestMemory(&irpMjRead, driverObj + DRIVER_OBJECT_MAJOR_FUNCTION + 
                sizeof(QWORD) * IRP_MJ_READ, sizeof(QWORD));
            Print("Found NtfsFsdRead at: %8, hooking now!\n", irpMjRead);
            ASSERT(HookingSetupGenericHook(irpMjRead, "NtfsFsdRead", FileHandleRead,
                FileHandleReadReturn) == STATUS_SUCCESS);
            Print("Removing NtReadFile...\n");
            ASSERT(HookingRemoveHook("NtReadFile") == STATUS_SUCCESS);
        }
    }
    // Get the MFTIndex of the current file
    ObjGetObjectField(FILE_OBJECT, fileObject, FILE_OBJECT_SCB, &scb);
    FileTranslateScbToFcb(scb, &fcb);
    FileGetFcbField(fcb, FCB_MFT_INDEX, &fileIndex);
    // Check if the current file is protected
    if(FileGetRuleByIndex(fileIndex, &hiddenFileRule) == STATUS_SUCCESS)
    {
        // The file is a protected file
        ObjGetCurrent_ETHREAD(&ethread);
        ObjGetObjectField(ETHREAD, ethread, ETHREAD_THREAD_ID, &threadId);
        WinHookReturnEvent(regs->rsp, threadId, context->virtualAddress);
        threadEvent = WinGetEventForThread(threadId);
        threadEvent->dataUnion.NtReadFile.rule = hiddenFileRule;
        threadEvent->dataUnion.NtReadFile.ioStatusBlock = params[4];
        threadEvent->dataUnion.NtReadFile.userBuffer = params[5];
    }
NtReadFileEmulateInstruction:
    // Emulate replaced instruction: mov rax,rsp
    regs->rax = regs->rsp;
    regs->rip += context->relatedConfig->instructionLength;
    // End emulation
    return STATUS_SUCCESS;
}

STATUS ShdHandleNtReadFileReturn(IN PHOOK_CONTEXT context)
{
    PCURRENT_GUEST_STATE state;
    PSHARED_CPU_DATA shared;
    PREGISTERS regs;
    QWORD threadId, ethread, bufferLength, idx, count, indecies[10];
    PHIDDEN_FILE_RULE rule;
    BYTE readDataBuffer[BUFF_MAX_SIZE];
    PWCHAR utf16Ptr;
    PTHREAD_EVENT threadEvent;
    STATUS status;

    state = VmmGetVmmStruct();
    shared = state->currentCPU->sharedData;
    regs = &state->guestRegisters;
    // Get thread id
    ObjGetCurrent_ETHREAD(&ethread);
    ObjGetObjectField(ETHREAD, ethread, ETHREAD_THREAD_ID, &threadId);
    // Using the thread id, get the saved data
    threadEvent = WinGetEventForThread(threadId);
    // Get the rule found in the hashmap
    rule = threadEvent->dataUnion.NtReadFile.rule;
    // Copy the readen data length (stored in the inforamtion member of IoStatusBlock)
    WinMmCopyGuestMemory(&bufferLength, threadEvent->dataUnion.NtReadFile.ioStatusBlock
     + sizeof(PVOID), sizeof(QWORD));
    if(!bufferLength)
        goto NtReadFilePutReturnAddress;
    // Copy the readon data itself
    WinMmCopyGuestMemory(readDataBuffer, threadEvent->dataUnion.NtReadFile.userBuffer, bufferLength);
    // Replace hidden content (if exist)
    if(bufferLength >= rule->content.length && 
        (count = MemoryContains(readDataBuffer, bufferLength, rule->content.data, rule->content.length, indecies)) 
            != 0)
    {
        if(rule->encoding == ENCODING_TYPE_UTF_16)
        {
            utf16Ptr = readDataBuffer;
            for(QWORD k = 0; k < count; k++)
                for(QWORD i = indecies[k] / 2; i < (indecies[k] + rule->content.length) / 2; i++)
                    utf16Ptr[i] = L'*';
        }
        else if(rule->encoding == ENCODING_TYPE_UTF_8)
            for(QWORD k = 0; k < count; k++)
                for(QWORD i = indecies[k]; i < indecies[k] + rule->content.length; i++)
                    readDataBuffer[i] = '*';
    }
    WinMmCopyMemoryToGuest(threadEvent->dataUnion.NtReadFile.userBuffer, readDataBuffer, bufferLength);
NtReadFilePutReturnAddress:
    // Put back the saved address in the RIP register
    regs->rip = threadEvent->returnAddress;
    return STATUS_SUCCESS;
}