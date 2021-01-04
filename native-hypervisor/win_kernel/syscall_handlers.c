#include <win_kernel/syscall_handlers.h>
#include <win_kernel/memory_manager.h>
#include <debug.h>
#include <vmm/vmcs.h>
#include <vmm/vm_operations.h>
#include <win_kernel/kernel_objects.h>
#include <vmx_modules/hooking_module.h>

static __attribute__((section(".nt_data"))) SYSCALL_DATA syscallsData[] = {  { NULL, 8 },  { NULL, 1 },  { NULL, 6 },  { NULL, 3 },  { NULL, 3 },  { NULL, 3 },  { NULL, 9 },  { NULL, 10 }, 
 { NULL, 9 },  { NULL, 5 },  { NULL, 3 },  { NULL, 4 },  { NULL, 2 },  { NULL, 4 },  { NULL, 2 },  { NULL, 1 }, 
 { NULL, 5 },  { NULL, 5 },  { NULL, 3 },  { NULL, 6 },  { NULL, 3 },  { NULL, 2 },  { NULL, 5 },  { NULL, 6 }, 
 { NULL, 6 },  { NULL, 5 },  { NULL, 5 },  { NULL, 9 },  { NULL, 4 },  { NULL, 7 },  { NULL, 4 },  { NULL, 2 }, 
 { NULL, 2 },  { NULL, 5 },  { NULL, 3 },  { NULL, 6 },  { NULL, 4 },  { NULL, 5 },  { NULL, 4 },  { NULL, 5 }, 
 { NULL, 10 },  { NULL, 11 },  { NULL, 2 },  { NULL, 5 },  { NULL, 2 },  { NULL, 1 },  { NULL, 9 },  { NULL, 5 }, 
 { NULL, 4 },  { NULL, 2 },  { NULL, 6 },  { NULL, 6 },  { NULL, 2 },  { NULL, 11 },  { NULL, 4 },  { NULL, 3 }, 
 { NULL, 5 },  { NULL, 10 },  { NULL, 5 },  { NULL, 3 },  { NULL, 7 },  { NULL, 2 },  { NULL, 1 },  { NULL, 5 }, 
 { NULL, 3 },  { NULL, 6 },  { NULL, 6 },  { NULL, 2 },  { NULL, 1 },  { NULL, 5 },  { NULL, 0 },  { NULL, 3 }, 
 { NULL, 5 },  { NULL, 5 },  { NULL, 7 },  { NULL, 2 },  { NULL, 2 },  { NULL, 9 },  { NULL, 8 },  { NULL, 2 }, 
 { NULL, 5 },  { NULL, 5 },  { NULL, 2 },  { NULL, 2 },  { NULL, 6 },  { NULL, 11 },  { NULL, 5 },  { NULL, 6 }, 
 { NULL, 3 },  { NULL, 16 },  { NULL, 1 },  { NULL, 5 },  { NULL, 4 },  { NULL, 2 },  { NULL, 4 },  { NULL, 5 }, 
 { NULL, 6 },  { NULL, 2 },  { NULL, 7 },  { NULL, 11 },  { NULL, 11 },  { NULL, 16 },  { NULL, 17 },  { NULL, 3 }, 
 { NULL, 4 },  { NULL, 2 },  { NULL, 2 },  { NULL, 6 },  { NULL, 16 },  { NULL, 2 },  { NULL, 1 },  { NULL, 1 }, 
 { NULL, 1 },  { NULL, 3 },  { NULL, 3 },  { NULL, 4 },  { NULL, 7 },  { NULL, 9 },  { NULL, 3 },  { NULL, 11 }, 
 { NULL, 11 },  { NULL, 3 },  { NULL, 6 },  { NULL, 4 },  { NULL, 3 },  { NULL, 3 },  { NULL, 3 },  { NULL, 3 }, 
 { NULL, 3 },  { NULL, 3 },  { NULL, 2 },  { NULL, 3 },  { NULL, 3 },  { NULL, 6 },  { NULL, 6 },  { NULL, 5 }, 
 { NULL, 6 },  { NULL, 3 },  { NULL, 8 },  { NULL, 4 },  { NULL, 2 },  { NULL, 2 },  { NULL, 8 },  { NULL, 4 }, 
 { NULL, 3 },  { NULL, 3 },  { NULL, 2 },  { NULL, 2 },  { NULL, 2 },  { NULL, 2 },  { NULL, 2 },  { NULL, 2 }, 
 { NULL, 2 },  { NULL, 2 },  { NULL, 2 },  { NULL, 3 },  { NULL, 1 },  { NULL, 1 },  { NULL, 8 },  { NULL, 4 }, 
 { NULL, 4 },  { NULL, 3 },  { NULL, 5 },  { NULL, 9 },  { NULL, 8 },  { NULL, 3 },  { NULL, 3 },  { NULL, 4 }, 
 { NULL, 3 },  { NULL, 3 },  { NULL, 8 },  { NULL, 4 },  { NULL, 9 },  { NULL, 8 },  { NULL, 4 },  { NULL, 14 }, 
 { NULL, 4 },  { NULL, 4 },  { NULL, 5 },  { NULL, 4 },  { NULL, 8 },  { NULL, 9 },  { NULL, 10 },  { NULL, 4 }, 
 { NULL, 7 },  { NULL, 5 },  { NULL, 4 },  { NULL, 11 },  { NULL, 4 },  { NULL, 5 },  { NULL, 13 },  { NULL, 17 }, 
 { NULL, 10 },  { NULL, 6 },  { NULL, 11 },  { NULL, 3 },  { NULL, 5 },  { NULL, 7 },  { NULL, 10 },  { NULL, 2 }, 
 { NULL, 3 },  { NULL, 1 },  { NULL, 1 },  { NULL, 1 },  { NULL, 1 },  { NULL, 1 },  { NULL, 3 },  { NULL, 1 }, 
 { NULL, 2 },  { NULL, 2 },  { NULL, 1 },  { NULL, 0 },  { NULL, 1 },  { NULL, 1 },  { NULL, 0 },  { NULL, 2 }, 
 { NULL, 2 },  { NULL, 3 },  { NULL, 5 },  { NULL, 2 },  { NULL, 5 },  { NULL, 6 },  { NULL, 14 },  { NULL, 5 }, 
 { NULL, 2 },  { NULL, 3 },  { NULL, 1 },  { NULL, 0 },  { NULL, 4 },  { NULL, 0 },  { NULL, 3 },  { NULL, 1 }, 
 { NULL, 2 },  { NULL, 6 },  { NULL, 6 },  { NULL, 2 },  { NULL, 0 },  { NULL, 1 },  { NULL, 2 },  { NULL, 3 }, 
 { NULL, 5 },  { NULL, 6 },  { NULL, 5 },  { NULL, 7 },  { NULL, 7 },  { NULL, 1 },  { NULL, 3 },  { NULL, 5 }, 
 { NULL, 3 },  { NULL, 1 },  { NULL, 4 },  { NULL, 0 },  { NULL, 0 },  { NULL, 2 },  { NULL, 1 },  { NULL, 9 }, 
 { NULL, 2 },  { NULL, 3 },  { NULL, 8 },  { NULL, 10 },  { NULL, 2 },  { NULL, 1 },  { NULL, 4 },  { NULL, 1 }, 
 { NULL, 1 },  { NULL, 5 },  { NULL, 6 },  { NULL, 3 },  { NULL, 9 },  { NULL, 1 },  { NULL, 1 },  { NULL, 9 }, 
 { NULL, 10 },  { NULL, 10 },  { NULL, 12 },  { NULL, 8 },  { NULL, 5 },  { NULL, 3 },  { NULL, 3 },  { NULL, 3 }, 
 { NULL, 4 },  { NULL, 4 },  { NULL, 5 },  { NULL, 3 },  { NULL, 3 },  { NULL, 12 },  { NULL, 3 },  { NULL, 4 }, 
 { NULL, 3 },  { NULL, 3 },  { NULL, 5 },  { NULL, 3 },  { NULL, 3 },  { NULL, 3 },  { NULL, 4 },  { NULL, 3 }, 
 { NULL, 5 },  { NULL, 6 },  { NULL, 3 },  { NULL, 2 },  { NULL, 2 },  { NULL, 2 },  { NULL, 2 },  { NULL, 3 }, 
 { NULL, 6 },  { NULL, 5 },  { NULL, 4 },  { NULL, 3 },  { NULL, 2 },  { NULL, 1 },  { NULL, 2 },  { NULL, 2 }, 
 { NULL, 2 },  { NULL, 10 },  { NULL, 7 },  { NULL, 2 },  { NULL, 9 },  { NULL, 2 },  { NULL, 5 },  { NULL, 5 }, 
 { NULL, 5 },  { NULL, 5 },  { NULL, 5 },  { NULL, 5 },  { NULL, 5 },  { NULL, 5 },  { NULL, 5 },  { NULL, 1 }, 
 { NULL, 2 },  { NULL, 5 },  { NULL, 5 },  { NULL, 6 },  { NULL, 5 },  { NULL, 2 },  { NULL, 4 },  { NULL, 0 }, 
 { NULL, 9 },  { NULL, 6 },  { NULL, 5 },  { NULL, 6 },  { NULL, 5 },  { NULL, 3 },  { NULL, 4 },  { NULL, 5 }, 
 { NULL, 6 },  { NULL, 3 },  { NULL, 6 },  { NULL, 5 },  { NULL, 6 },  { NULL, 3 },  { NULL, 6 },  { NULL, 2 }, 
 { NULL, 2 },  { NULL, 1 },  { NULL, 1 },  { NULL, 5 },  { NULL, 1 },  { NULL, 4 },  { NULL, 1 },  { NULL, 6 }, 
 { NULL, 2 },  { NULL, 2 },  { NULL, 2 },  { NULL, 3 },  { NULL, 3 },  { NULL, 2 },  { NULL, 2 },  { NULL, 2 }, 
 { NULL, 3 },  { NULL, 3 },  { NULL, 1 },  { NULL, 0 },  { NULL, 2 },  { NULL, 2 },  { NULL, 2 },  { NULL, 2 }, 
 { NULL, 2 },  { NULL, 2 },  { NULL, 3 },  { NULL, 3 },  { NULL, 9 },  { NULL, 0 },  { NULL, 2 },  { NULL, 2 }, 
 { NULL, 5 },  { NULL, 6 },  { NULL, 2 },  { NULL, 3 },  { NULL, 1 },  { NULL, 2 },  { NULL, 1 },  { NULL, 2 }, 
 { NULL, 4 },  { NULL, 1 },  { NULL, 1 },  { NULL, 2 },  { NULL, 5 },  { NULL, 4 },  { NULL, 4 },  { NULL, 4 }, 
 { NULL, 4 },  { NULL, 4 },  { NULL, 4 },  { NULL, 4 },  { NULL, 4 },  { NULL, 6 },  { NULL, 4 },  { NULL, 2 }, 
 { NULL, 5 },  { NULL, 6 },  { NULL, 6 },  { NULL, 1 },  { NULL, 1 },  { NULL, 4 },  { NULL, 3 },  { NULL, 2 }, 
 { NULL, 5 },  { NULL, 3 },  { NULL, 3 },  { NULL, 2 },  { NULL, 2 },  { NULL, 4 },  { NULL, 4 },  { NULL, 3 }, 
 { NULL, 1 },  { NULL, 5 },  { NULL, 1 },  { NULL, 1 },  { NULL, 2 },  { NULL, 4 },  { NULL, 2 },  { NULL, 1 }, 
 { NULL, 1 },  { NULL, 4 },  { NULL, 1 },  { NULL, 2 },  { NULL, 6 },  { NULL, 2 },  { NULL, 2 },  { NULL, 0 }, 
 { NULL, 0 },  { NULL, 0 },  { NULL, 6 },  { NULL, 4 },  { NULL, 1 },  { NULL, 1 },  { NULL, 1 },  { NULL, 2 }, 
 { NULL, 2 },  { NULL, 5 },  { NULL, 4 },  { NULL, 3 },  { NULL, 1 },  { NULL, 7 },  { NULL, 2 },  { NULL, 2 }, 
 { NULL, 4 },  { NULL, 4 },  { NULL, 5 },  { NULL, 1 },  { NULL, 1 } };

static __attribute__((section(".nt_sycalls_events"))) SYSCALL_EVENT syscallEvents[25000];

VOID ShdInitSyscallData(IN QWORD syscallId, IN BYTE hookInstructionOffset, IN BYTE hookedInstructionLength,
    IN SYSCALL_HANDLER handler, IN BOOL hookReturn, IN SYSCALL_HANDLER returnHandler)
{
    syscallsData[syscallId].hookInstructionOffset = hookInstructionOffset;
    syscallsData[syscallId].hookedInstructionLength = hookedInstructionLength;
    syscallsData[syscallId].hookReturnEvent = hookReturn;
    syscallsData[syscallId].handler = handler;
    syscallsData[syscallId].returnHandler = returnHandler;
}

VOID ShdGetParameters(OUT QWORD_PTR params, IN BYTE count)
{
    PREGISTERS regs = &VmmGetVmmStruct()->guestRegisters;
    QWORD paramsStart = regs->rsp + 5 * sizeof(QWORD);
    switch(count)
    {
        case 17:
        case 16:
        case 15:
        case 14:
        case 13:
        case 12:
        case 11:
        case 10:
        case 9:
        case 8:
        case 7:
        case 6:
        case 5:
            WinMmCopyGuestMemory(params + 4, paramsStart, (count - 4) * sizeof(QWORD));
        case 4:
            params[3] = regs->r9;
        case 3:
            params[2] = regs->r8;
        case 2:
            params[1] = regs->rdx;
        case 1:
            params[0] = regs->rcx;
    }
}

VOID ShdHookReturnEvent(IN QWORD syscallId, IN QWORD rsp, IN QWORD threadId)
{
    QWORD returnAddress;
    
    returnAddress = CALC_RETURN_HOOK_ADDR(syscallsData[syscallId].virtualHookedInstructionAddress);
    WinMmCopyGuestMemory(&syscallEvents[threadId].returnAddress, rsp, sizeof(QWORD));
    WinMmCopyMemoryToGuest(rsp, &returnAddress, sizeof(QWORD));
}

STATUS ShdHandleNtOpenPrcoess()
{
    PCURRENT_GUEST_STATE state;
    PSHARED_CPU_DATA shared;
    PREGISTERS regs;

    state = VmmGetVmmStruct();
    shared = state->currentCPU->sharedData;
    regs = &state->guestRegisters;
    // Emulate replaced instruction: sub rsp,38h
    regs->rsp -= 0x38;
    regs->rip += syscallsData[NT_OPEN_PROCESS].hookedInstructionLength;
    // End emulation
    return STATUS_SUCCESS;
}

STATUS ShdHandleNtOpenPrcoessReturn()
{
    PCURRENT_GUEST_STATE state;
    PSHARED_CPU_DATA shared;
    PREGISTERS regs;

    state = VmmGetVmmStruct();
    shared = state->currentCPU->sharedData;
    regs = &state->guestRegisters;
    return STATUS_SUCCESS;
}

STATUS ShdHandleNtCreateUserProcess()
{
    PCURRENT_GUEST_STATE state;
    PSHARED_CPU_DATA shared;
    PREGISTERS regs;

    state = VmmGetVmmStruct();
    shared = state->currentCPU->sharedData;
    regs = &state->guestRegisters;
    // Emulate replaced instruction: push rbp
    ASSERT(WinMmCopyMemoryToGuest(regs->rsp - 8, &regs->rbp, sizeof(QWORD)) == STATUS_SUCCESS);
    regs->rsp -= 8;
    regs->rip += syscallsData[NT_CREATE_USER_PROCESS].hookedInstructionLength;
    // End emulation
    return STATUS_SUCCESS;
}

STATUS ShdHandleNtReadFile()
{
    PCURRENT_GUEST_STATE state;
    PSHARED_CPU_DATA shared;
    PMODULE module;
    PREGISTERS regs;
    PHOOKING_MODULE_EXTENSION ext;
    PQWORD_MAP filesData;
    QWORD params[17];
    QWORD fileObject, handleTable, eprocess, threadId, ethread, returnAddress, scb, fcb, fileIndex;
    PHIDDEN_FILE_RULE hiddenFileRule;
    WORD fileType;
    STATUS status;

    state = VmmGetVmmStruct();
    shared = state->currentCPU->sharedData;
    module = shared->staticVariables.handleNtReadFile.staticContent.handleNtReadFile.module;
    regs = &state->guestRegisters;
    // First get the syscalls module pointer
    if(!module)
    {
        if((status = MdlGetModuleByName(&module, "Windows Hooking Module")) != STATUS_SUCCESS)
        {
            Print("Could not find the desired module\n");
            goto NtReadFileEmulateInstruction;
        }
        shared->staticVariables.handleNtReadFile.staticContent.handleNtReadFile.module = module;
    }
    ext = module->moduleExtension;
    filesData = &ext->filesData;
    // Receive syscall parameters
    ShdGetParameters(params, syscallsData[NT_READ_FILE].params);
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
    // Get the MFTIndex of the current file
    ObjGetObjectField(FILE_OBJECT, fileObject, FILE_OBJECT_SCB, &scb);
    FileTranslateScbToFcb(scb, &fcb);
    FileGetFcbField(fcb, FCB_MFT_INDEX, &fileIndex);
    // Check if the current file is protected
    if((hiddenFileRule = MapGet(filesData, fileIndex)) != MAP_KEY_NOT_FOUND)
    {
        // The file is a protected file
        ObjGetCurrent_ETHREAD(&ethread);
        ObjGetObjectField(ETHREAD, ethread, ETHREAD_THREAD_ID, &threadId);
        ShdHookReturnEvent(NT_READ_FILE, regs->rsp, threadId);
        syscallEvents[threadId].dataUnion.NtReadFile.rule = hiddenFileRule;
        syscallEvents[threadId].dataUnion.NtReadFile.ioStatusBlock = params[4];
        syscallEvents[threadId].dataUnion.NtReadFile.userBuffer = params[5];
    }
NtReadFileEmulateInstruction:
    // Emulate replaced instruction: mov rax,rsp
    regs->rax = regs->rsp;
    regs->rip += syscallsData[NT_READ_FILE].hookedInstructionLength;
    // End emulation
    return STATUS_SUCCESS;
}

STATUS ShdHandleNtReadFileReturn()
{
    PCURRENT_GUEST_STATE state;
    PSHARED_CPU_DATA shared;
    PMODULE module;
    PREGISTERS regs;
    QWORD threadId, ethread, bufferLength, idx, pid, eprocess;
    PHIDDEN_FILE_RULE rule;
    BYTE readDataBuffer[BUFF_MAX_SIZE];
    STATUS status;

    state = VmmGetVmmStruct();
    shared = state->currentCPU->sharedData;
    module = shared->staticVariables.handleNtReadFileReturn.staticContent.handleNtReadFileReturn.module;
    regs = &state->guestRegisters;
    // First get the syscalls module pointer
    if(!module)
    {
        if((status = MdlGetModuleByName(&module, "Windows Hooking Module")) != STATUS_SUCCESS)
        {
            Print("Could not find the desired module\n");
            return status;
        }
        shared->staticVariables.handleNtReadFileReturn.staticContent.handleNtReadFileReturn
            .module = module;
    }
    ObjGetCurrent_ETHREAD(&ethread);
    ObjGetCurrent_EPROCESS(&eprocess);
    ObjGetObjectField(EPROCESS, eprocess, EPROCESS_PID, &pid);
    ObjGetObjectField(ETHREAD, ethread, ETHREAD_THREAD_ID, &threadId);
    Print("Process %d, Thread %d hooken the return event of NtReadFile\n", pid, threadId);
    // Get the rule found in the hashmap
    rule = syscallEvents[threadId].dataUnion.NtReadFile.rule;
    // Copy the readen data length (stored in the inforamtion member of IoStatusBlock)
    WinMmCopyGuestMemory(&bufferLength, syscallEvents[threadId].dataUnion.NtReadFile.ioStatusBlock + sizeof(PVOID),
        sizeof(QWORD));
    Print("The size of the data returned in buffer is %d\n", bufferLength);
    // Copy the readon data itself
    WinMmCopyGuestMemory(readDataBuffer, syscallEvents[threadId].dataUnion.NtReadFile.userBuffer, bufferLength);
    Print("Data: %.b", bufferLength, readDataBuffer);
    // Replace hidden content (if exist)
    if(bufferLength >= rule->content.length && (idx = MemoryContains(readDataBuffer, bufferLength, rule->content.data,
        rule->content.length)) != IDX_NOT_FOUND)
    {
        for(QWORD i = idx; i < idx + rule->content.length; i++)
            readDataBuffer[i] = L'0';
    }
    WinMmCopyMemoryToGuest(syscallEvents[threadId].dataUnion.NtReadFile.userBuffer, readDataBuffer, bufferLength);
    // Put back the saved address in the RIP register
    regs->rip = syscallEvents[threadId].returnAddress;
    return STATUS_SUCCESS;
}