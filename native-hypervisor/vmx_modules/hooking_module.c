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

STATUS HookingModuleInitializeAllCores(IN PSHARED_CPU_DATA sharedData, IN PMODULE module, IN PGENERIC_MODULE_DATA initData)
{
    PHOOKING_MODULE_EXTENSION extension;

    PrintDebugLevelDebug("Starting initialization of hooking module for all cores\n");
    sharedData->heap.allocate(&sharedData->heap, sizeof(HOOKING_MODULE_EXTENSION), &module->moduleExtension);
    HwSetMemory(module->moduleExtension, 0, sizeof(HOOKING_MODULE_EXTENSION));
    extension = module->moduleExtension;
    extension->startExitCount = FALSE;
    extension->exitCount = 0;
    extension->syscallsData = &__ntDataStart;
    MapCreate(&extension->addressToContext, BasicHashFunction, BASIC_HASH_LEN, DefaultEqualityFunction);
    /* System calls data initialization - START */
    // Init NtOpenProcess related data
    ShdInitSyscallData(NT_OPEN_PROCESS, 0, 4, ShdHandleNtOpenPrcoess, FALSE, NULL);
    ShdInitSyscallData(NT_CREATE_USER_PROCESS, 0, 2, ShdHandleNtCreateUserProcess, FALSE, NULL);
    ShdInitSyscallData(NT_READ_FILE, 0, 3, ShdHandleNtReadFile, TRUE, ShdHandleNtReadFileReturn);
    /* System calls data initialization - END */
    PrintDebugLevelDebug("Shared cores data successfully initialized for hooking module\n");
    return STATUS_SUCCESS;
}

STATUS HookingModuleInitializeSingleCore(IN PSINGLE_CPU_DATA data)
{
    PrintDebugLevelDebug("Starting initialization of hooking module on core #%d\n", data->coreIdentifier);
    // Hook the event of writing to the LSTAR MSR
    VmmUpdateMsrAccessPolicy(data, MSR_IA32_LSTAR, FALSE, TRUE);
    __vmwrite(EXCEPTION_BITMAP, vmread(EXCEPTION_BITMAP) | (1 << INT_BREAKPOINT));
    PrintDebugLevelDebug("Finished initialization of hooking module on core #%d\n", data->coreIdentifier);
    return STATUS_SUCCESS;
}

STATUS HookingDefaultHandler(IN PCURRENT_GUEST_STATE sharedData, IN PMODULE module)
{
    PHOOKING_MODULE_EXTENSION ext;
    BYTE_PTR ssdt, ntoskrnl, win32k;

    ext = (PHOOKING_MODULE_EXTENSION)module->moduleExtension;
    if(ext->exitCount++ >= COUNT_UNTIL_HOOK)
    {
        // perform lock-checking
        module->hasDefaultHandler = FALSE;
        HookingLocateSSDT(ext->lstar, &ssdt, ext->guestCr3);
        HookingGetSystemTables(ssdt, &ext->ntoskrnl, &ext->win32k, ext->guestCr3);
        ASSERT(HookingHookSystemCalls(module, ext->guestCr3, ext->ntoskrnl, ext->win32k, 1, NT_READ_FILE) 
            == STATUS_SUCCESS);
        Print("System calls were successfully hooked\n");
        return STATUS_SUCCESS;
    }
NotHandled:
    return STATUS_VM_EXIT_NOT_HANDLED;
}

STATUS HookingLocateSSDT(IN BYTE_PTR lstar, OUT BYTE_PTR* ssdt, IN QWORD guestCr3)
{
    // Pattern:
    // mov edi,eax
    // shr edi,7
    // and edi,20h
    // and eax,0FFFh
    BYTE pattern[] = { 0x8B, 0xF8, 0xC1, 0xEF, 0x07, 0x83, 0xE7, 0x20, 0x25, 0xFF, 0x0F, 0x00, 0x00 };
    BYTE kernelChunk[13];
    BYTE_PTR patternAddress;
    QWORD offset;

    offset = 0x60C159; // MS updates ntoskrnl.exe's image from time to time. Previouse values are: 0x60D359, 0x60C759;
    PrintDebugLevelDebug("Starting to search for the pattern: %.b in kernel's address space\n", 13, pattern);

    for(; offset < 0xffffffff; offset++)
    {
        if(WinMmCopyGuestMemory(kernelChunk, lstar - offset, 13) != STATUS_SUCCESS)
            continue;
        if(!HwCompareMemory(kernelChunk, pattern, 13))
        {
            Print("Pattern found in kernel's address space %8\n", offset);
            patternAddress = lstar - offset;
            goto SSDTFound;
        }
    }
    Print("Pattern was NOT found in kernel's address space\n");
    return STATUS_SSDT_NOT_FOUND;

SSDTFound:
    patternAddress += 13; // pattern
    patternAddress += 7;  // lea r10,[nt!KeServiceDescriptorTable]
    ASSERT(WinMmCopyGuestMemory(&offset, patternAddress + 3, sizeof(DWORD)) == STATUS_SUCCESS);
    *ssdt = (patternAddress + 7) + offset;
    return STATUS_SUCCESS;
}

VOID HookingGetSystemTables(IN BYTE_PTR ssdt, OUT BYTE_PTR* ntoskrnl, OUT BYTE_PTR* win32k, IN QWORD guestCr3)
{
    ASSERT(WinMmCopyGuestMemory(ntoskrnl, ssdt, sizeof(QWORD)) == STATUS_SUCCESS);
    ASSERT(WinMmCopyGuestMemory(win32k, ssdt + 32, sizeof(QWORD)) == STATUS_SUCCESS);
}

STATUS HookingHookSystemCalls(IN PMODULE module, IN QWORD guestCr3, IN BYTE_PTR ntoskrnl, IN BYTE_PTR win32k, 
    IN QWORD count, ...)
{
    va_list args;
    PSHARED_CPU_DATA shared;
    PHOOKING_MODULE_EXTENSION ext;
    QWORD syscallId, functionAddress, virtualFunctionAddress, physicalHookAddress, virtualHookAddress;
    DWORD offset;
    BYTE hookInstruction[X86_MAX_INSTRUCTION_LEN];
    BYTE hookedInstruction[X86_MAX_INSTRUCTION_LEN];
    PHOOK_CONTEXT hookContext;
    PHEAP heap;
    STATUS status;

    ext = module->moduleExtension;
    va_start(args, count);
    shared = VmmGetVmmStruct()->currentCPU->sharedData;
    heap = &shared->heap;
    while(count--)
    {
        // Get the syscall id from va_arg
        syscallId = va_arg(args, QWORD);
        // Get the offset of the syscall handler (in ntoskrnl.exe) from the shadowed SSDT
        ASSERT(WinMmCopyGuestMemory(&offset, ntoskrnl + syscallId * sizeof(DWORD), 
            sizeof(DWORD)) == STATUS_SUCCESS);
        // Get the guest physical address of the syscall handler
        virtualFunctionAddress = ntoskrnl + (offset >> 4);
        ASSERT(WinMmTranslateGuestVirtualToGuestPhysical(ntoskrnl + (offset >> 4), &functionAddress) == STATUS_SUCCESS);
        Print("Syscall ID: %d, Virtual: %8, Guest Physical: %8\n", syscallId, ntoskrnl + (offset >> 4),
             functionAddress);
        // Save hook information in system calls database
        physicalHookAddress = functionAddress + ext->syscallsData[syscallId].hookInstructionOffset;
        virtualHookAddress = virtualFunctionAddress + ext->syscallsData[syscallId].hookInstructionOffset;
        ext->syscallsData[syscallId].hookedInstructionAddress = physicalHookAddress;
        ext->syscallsData[syscallId].virtualHookedInstructionAddress = virtualHookAddress;
        HwCopyMemory(hookedInstruction, WinMmTranslateGuestPhysicalToHostVirtual(physicalHookAddress),
            ext->syscallsData[syscallId].hookedInstructionLength);
        // Build the hook instruction ((INT3)(INT3-OPTIONAL)(NOP)(NOP)(NOP)(NOP)...)
        hookInstruction[0] = INT3_OPCODE; hookInstruction[1] = INT3_OPCODE;
        HwSetMemory(hookInstruction + 2, NOP_OPCODE, ext->syscallsData[syscallId].hookedInstructionLength - 2);
        // Inject the hooked instruction to the guest and print current stored instruction at address
        Print("Injecting a hook instruction of length %d to %8. Current instruction is: %.b\n", 
                ext->syscallsData[syscallId].hookedInstructionLength,
                physicalHookAddress,
                ext->syscallsData[syscallId].hookedInstructionLength,
                WinMmTranslateGuestPhysicalToHostVirtual(physicalHookAddress)
             );
        HwCopyMemory(WinMmTranslateGuestPhysicalToHostVirtual(physicalHookAddress), hookInstruction, 
            ext->syscallsData[syscallId].hookedInstructionLength);
        // Translate address to hook context
        if((status = heap->allocate(heap, sizeof(HOOK_CONTEXT), &hookContext)) != STATUS_SUCCESS)
        {
            Print("Could not allocate memory for hook context\n");
            return status;
        }
        hookContext->handler = ext->syscallsData[syscallId].handler;
        hookContext->additionalData = syscallId;
        MapSet(&ext->addressToContext, physicalHookAddress, hookContext);
        if(ext->syscallsData[syscallId].hookReturnEvent)
        {
            if((status = heap->allocate(heap, sizeof(HOOK_CONTEXT), &hookContext)) != STATUS_SUCCESS)
            {
                Print("Could not allocate memory for hook context\n");
                return status;
            }
            hookContext->handler = ext->syscallsData[syscallId].returnHandler;
            hookContext->additionalData = syscallId;
            MapSet(&ext->addressToContext, CALC_RETURN_HOOK_ADDR(physicalHookAddress), hookContext);
        }
        ASSERT(KppAddNewEntry(physicalHookAddress, ext->syscallsData[syscallId].hookedInstructionLength, hookedInstruction) 
            == STATUS_SUCCESS);
    }
    va_end(args);
    return STATUS_SUCCESS;
}

STATUS HookingHandleMsrWrite(IN PCURRENT_GUEST_STATE data, IN PMODULE module)
{
    // PatchGaurd might put a fake LSTAR value later, hence we save it now
    PREGISTERS regs;
    QWORD msrValue;
    PHOOKING_MODULE_EXTENSION ext;

    regs = &data->guestRegisters;
    if(regs->rcx != MSR_IA32_LSTAR)
        return STATUS_VM_EXIT_NOT_HANDLED;
    msrValue = ((regs->rdx & 0xffffffff) << 32) | (regs->rax & 0xffffffff);
    Print("Guest attempted to write to LSTAR %8 MSR: %8\n", regs->rcx, msrValue);
    __writemsr(MSR_IA32_LSTAR, msrValue);
    regs->rip += vmread(VM_EXIT_INSTRUCTION_LEN);
    ext = module->moduleExtension;
    ext->lstar = msrValue;
    ext->startExitCount = TRUE;
    /* 
        Due to Meltdown mitigations, the address might be not mapped. 
        Hence, we are saving the current CR3 for future usage.
    */
    ext->guestCr3 = vmread(GUEST_CR3);
    return STATUS_SUCCESS;
}

STATUS HookingHandleException(IN PCURRENT_GUEST_STATE data, IN PMODULE module)
{
    BYTE vector;
    QWORD syscallId, ripPhysicalAddress;
    PHOOKING_MODULE_EXTENSION ext;
    PHOOK_CONTEXT hookContext;

    vector = vmread(VM_EXIT_INTR_INFO) & 0xff;
    if(vector != INT_BREAKPOINT)
        return STATUS_VM_EXIT_NOT_HANDLED;
    
    ext = module->moduleExtension;
    ASSERT(WinMmTranslateGuestVirtualToGuestPhysical(data->guestRegisters.rip, &ripPhysicalAddress)
        == STATUS_SUCCESS);
    if((hookContext = MapGet(&ext->addressToContext, ripPhysicalAddress)) != MAP_KEY_NOT_FOUND)
        ASSERT(hookContext->handler() == STATUS_SUCCESS);
    else
        VmmInjectGuestInterrupt(INT_BREAKPOINT, 0);
    return STATUS_SUCCESS;
}