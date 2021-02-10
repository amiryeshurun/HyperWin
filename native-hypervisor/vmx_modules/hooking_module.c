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
#include <vmm/exit_reasons.h>

STATUS HookingModuleInitializeAllCores(IN PMODULE module)
{
    PHOOKING_MODULE_EXTENSION extension;
    PSHARED_CPU_DATA sharedData;

    sharedData = VmmGetVmmStruct()->currentCPU->sharedData;
    PrintDebugLevelDebug("Starting initialization of hooking module for all cores\n");
    // Init the new module
    MdlInitModule(module);
    // Set name
    MdlSetModuleName(module, HOOKING_MODULE_NAME);
    // Register VM-Exit handlers
    MdlRegisterVmExitHandler(module, EXIT_REASON_MSR_WRITE, HookingHandleMsrWrite);
    MdlRegisterVmExitHandler(module, EXIT_REASON_EXCEPTION_NMI, HookingHandleException);
    // Register the module
    MdlRegisterModule(module);
    // Set default handler
    module->hasDefaultHandler = TRUE;
    module->defaultHandler = HookingDefaultHandler;
    // Allocate space for extension
    sharedData->heap.allocate(&sharedData->heap, sizeof(HOOKING_MODULE_EXTENSION), &module->moduleExtension);
    HwSetMemory(module->moduleExtension, 0, sizeof(HOOKING_MODULE_EXTENSION));
    // Init extension
    extension = module->moduleExtension;
    extension->startExitCount = FALSE;
    extension->exitCount = 0;
    extension->hookingConfigSegment = PhysicalToVirtual(&__hooking_config_segment);
    ListCreate(&extension->hookConfig);
    MapCreate(&extension->addressToContext, BasicHashFunction, BASIC_HASH_LEN, DefaultEqualityFunction);
    // Parse config segment, assert if failed
    ASSERT(HookingParseConfig(extension->hookingConfigSegment, &extension->hookConfig) == STATUS_SUCCESS);
    PrintDebugLevelDebug("Shared cores data successfully initialized for hooking module\n");

    return STATUS_SUCCESS;
}

STATUS HookingModuleInitializeSingleCore(IN PMODULE module)
{
    PSINGLE_CPU_DATA data;

    data = VmmGetVmmStruct()->currentCPU;
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
    static SPIN_LOCK lock = SPIN_LOCK_INIT;

    ext = (PHOOKING_MODULE_EXTENSION)module->moduleExtension;
    // All cores must wait until data is successfully hooked
    SPIN_LOCK(&lock);
    if(ext->exitCount++ >= COUNT_UNTIL_HOOK)
    {
        module->hasDefaultHandler = FALSE;
        module->isHandledOnVmExit[EXIT_REASON_MSR_WRITE] = FALSE;
        for (QWORD i = 0; i < sharedData->currentCPU->sharedData->numberOfCores; i++)
            VmmUpdateMsrAccessPolicy(sharedData->currentCPU->sharedData->cpuData[i], MSR_IA32_LSTAR, FALSE, TRUE);
        HookingLocateSSDT(ext->lstar, &ssdt, ext->guestCr3);
        HookingGetSystemTables(ssdt, &ext->ntoskrnl, &ext->win32k, ext->guestCr3);
        ASSERT(HookingHookSystemCalls(ext->guestCr3, ext->ntoskrnl, ext->win32k, 1, NT_READ_FILE, 
            ShdHandleNtReadFile, NULL) == STATUS_SUCCESS);
        Print("System calls were successfully hooked\n");
        return STATUS_SUCCESS;
    }
    SPIN_UNLOCK(&lock);

NotHandled:
    return STATUS_VM_EXIT_NOT_HANDLED;
}

STATUS HookingTranslateSyscallNameToId(IN PCHAR name, OUT QWORD_PTR syscallId)
{
    PLIST hookConfig;
    PLIST_ENTRY head;
    PSYSCALL_CONFIG_HOOK_CONTEXT syscallConfigContext;
    PCONFIG_HOOK_CONTEXT configContext;
    PHOOKING_MODULE_EXTENSION ext;
    QWORD nameLength;
    static PMODULE module;
    
    // First get the module
    if(!module)
        MdlGetModuleByName(&module, HOOKING_MODULE_NAME);
    // Get the module extension
    ext = module->moduleExtension;
    // Get the list of config entries
    hookConfig = &ext->hookConfig;
    head = hookConfig->head;
    // Get the name
    nameLength = StringLength(name);
    while(head)
    {
        configContext = (PCONFIG_HOOK_CONTEXT)head->data;
        if(configContext->type == HOOK_TYPE_SYSCALL && 
            !HwCompareMemory(configContext->name, name, nameLength))
        {
            syscallConfigContext = (PSYSCALL_CONFIG_HOOK_CONTEXT)configContext->additionalData;
            *syscallId = syscallConfigContext->syscallId;
            return STATUS_SUCCESS;
        }
        head = head->next;
    }

    return STATUS_SYSCALL_NOT_FOUND;
}

STATUS HookingParseConfig(IN BYTE_PTR hookConfigSegment, IN PLIST hookConfig)
{
    BYTE_PTR current;
    QWORD tokenLength, syscallId, offset, instructionLength;
    PCHAR name;
    PCONFIG_HOOK_CONTEXT configContext;
    PSYSCALL_CONFIG_HOOK_CONTEXT syscallConfigContext;
    PHEAP heap;
    BYTE type;
    STATUS status = STATUS_SUCCESS;

    current = hookConfigSegment;
    heap = &VmmGetVmmStruct()->currentCPU->sharedData->heap;
    while(TRUE)
    {
        if(!HwCompareMemory(current, "END", 3))
            break;
        // Get the name
        tokenLength = GetTokenLength(current, ',');
        SUCCESS_OR_CLEANUP(heap->allocate(heap, (tokenLength + 1) * sizeof(char), &name));
        HwCopyMemory(name, current, tokenLength);
        name[tokenLength] = '\0';
        current += (tokenLength + 1);
        // Get the hook type
        tokenLength = GetTokenLength(current, ',');
        if(!HwCompareMemory(current, "SYSCALL", tokenLength))
            type = HOOK_TYPE_SYSCALL;
        else if(!HwCompareMemory(current, "GENERIC", tokenLength))
            type = HOOK_TYPE_GENERIC;
        else
            return STATUS_UNKNOWN_HOOK_TYPE;
        current += (tokenLength + 1);
        // Allocate space for hook context config
        SUCCESS_OR_CLEANUP(heap->allocate(heap, sizeof(CONFIG_HOOK_CONTEXT), &configContext));
        configContext->name = name;
        configContext->type = type;
        // Get the specific-type data
        switch(type)
        {
            case HOOK_TYPE_SYSCALL:
            {
                // Load syscall ID
                tokenLength = GetTokenLength(current, ',');
                syscallId = StringToInt(current, tokenLength);
                current += (tokenLength + 1);
                // Allocate space for a specific-type config data
                syscallConfigContext = NULL;
                SUCCESS_OR_CLEANUP(heap->allocate(heap, sizeof(SYSCALL_CONFIG_HOOK_CONTEXT), 
                    &syscallConfigContext));
                // Store syscall ID & Save the context pointer
                syscallConfigContext->syscallId = syscallId;
                configContext->additionalData = syscallConfigContext;
                break;
            }
        }
        // Get params & offset & instruction length (ordered)
        tokenLength = GetTokenLength(current, ',');
        configContext->params = StringToInt(current, tokenLength);
        current += (tokenLength + 1);
        tokenLength = GetTokenLength(current, ',');
        configContext->offsetFromBeginning = StringToInt(current, tokenLength);
        current += (tokenLength + 1);
        tokenLength = GetTokenLength(current, ',');
        configContext->instructionLength =  StringToInt(current, tokenLength);
        // Skip \r\n
        current += (tokenLength + 2);
        // Print to log
        Print("==========\nScanned a config field with the following data:\nName: ");
        Print(name);
        Print("\nType: %d\nParams: %d\nOffset: %d\nInstruction Length: %d\n==========\n",
            configContext->type, 
            configContext->params,
            configContext->offsetFromBeginning,
            configContext->instructionLength);
        switch(type)
        {
            case HOOK_TYPE_SYSCALL:
            {
                Print("System Call ID: %d\n", syscallConfigContext->syscallId);
                break;
            }
        }
        // Now configContext is ready to be stored in the list
        ListInsert(hookConfig, configContext);
    }

cleanup:
    if(status && name)
        heap->deallocate(heap, name);
    if(status && configContext)
        heap->deallocate(heap, configContext);
    if(status && syscallConfigContext)
        heap->deallocate(heap, syscallConfigContext);
    return status;
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

STATUS HookingHookSystemCalls(IN QWORD guestCr3, IN BYTE_PTR ntoskrnl, IN BYTE_PTR win32k, 
    IN QWORD count, ...)
{
    va_list args;
    QWORD syscallId, functionAddress, virtualFunctionAddress;
    DWORD offset;
    PCHAR syscallName;
    PVOID handler;
    PVOID returnHandler;
    STATUS status = STATUS_SUCCESS;

    va_start(args, count);
    while(count--)
    {
        // Get the syscall name and handlers from va_arg
        syscallName = va_arg(args, PCHAR);
        handler = va_arg(args, PVOID);
        returnHandler = va_arg(args, PVOID);
        // Translate name to id
        SUCCESS_OR_CLEANUP(HookingTranslateSyscallNameToId(syscallName, &syscallId));
        // Get the offset of the syscall handler (in ntoskrnl.exe) from the shadowed SSDT
        ASSERT(WinMmCopyGuestMemory(&offset, ntoskrnl + syscallId * sizeof(DWORD), 
            sizeof(DWORD)) == STATUS_SUCCESS);
        // Get the guest physical address of the syscall handler
        virtualFunctionAddress = ntoskrnl + (offset >> 4);
        ASSERT(WinMmTranslateGuestVirtualToGuestPhysical(virtualFunctionAddress, &functionAddress) == STATUS_SUCCESS);
        Print("Syscall ID: %d, Virtual: %8, Guest Physical: %8\n", syscallId, virtualFunctionAddress,
             functionAddress);
        // Set the hook
        ASSERT(HookingSetupGenericHook(virtualFunctionAddress,
                syscallName,
                handler,
                returnHandler) == STATUS_SUCCESS);
    }
    va_end(args);

cleanup:
    return status;
}

STATUS HookingSetupGenericHook(IN QWORD guestVirtualAddress, IN PCHAR name, IN HOOK_HANDLER handler,
    IN HOOK_HANDLER returnHandler)
{
    PHEAP heap;
    PHOOK_CONTEXT hookContext;
    PCONFIG_HOOK_CONTEXT configContext;
    PSYSCALL_CONFIG_HOOK_CONTEXT syscallConfigContext;
    PLIST_ENTRY configHead;
    PHOOKING_MODULE_EXTENSION ext;
    QWORD guestPhysicalAddress, instructionLength;
    static PMODULE hookingModule;
    BYTE hookedInstruction[X86_MAX_INSTRUCTION_LEN], hookInstruction[X86_MAX_INSTRUCTION_LEN];
    STATUS status = STATUS_SUCCESS;

    if(hookInstruction <= 1)
        return STATUS_INSTRUCTION_TOO_SHORT;
    // First get module and extension
    if(!hookingModule)
        SUCCESS_OR_CLEANUP(MdlGetModuleByName(&hookingModule, HOOKING_MODULE_NAME));
    ext = hookingModule->moduleExtension;
    // Allocate space for hook context
    heap = &VmmGetVmmStruct()->currentCPU->sharedData->heap;
    // Find the config entry for the specified hook name
    configHead = ext->hookConfig.head;
    while(configHead)
    {
        configContext = (PCONFIG_HOOK_CONTEXT)configHead->data;
        if(!HwCompareMemory(configContext->name, name, StringLength(name)))
            break;
        configHead = configHead->next;
    }
    if(!configContext)
        return STATUS_UNKNOWN_HOOK_NAME;
    // Get data from hook context
    guestVirtualAddress += configContext->offsetFromBeginning;
    instructionLength = configContext->instructionLength;
    // If an entry was found, allocate space for hook context
    SUCCESS_OR_CLEANUP(heap->allocate(heap, sizeof(HOOK_CONTEXT), &hookContext));
    // Set handler
    hookContext->handler = handler;
    // Calculate address of hook
    SUCCESS_OR_CLEANUP(WinMmTranslateGuestVirtualToGuestPhysical(guestVirtualAddress, &guestPhysicalAddress));
    // Save the related config information
    hookContext->relatedConfig = configContext;
    hookContext->virtualAddress = guestVirtualAddress;
    // Save the physical address of hook
    configContext->guestPhysicalAddress = guestPhysicalAddress;
    // Finally, set hook after saving all of its data
    MapSet(&ext->addressToContext, guestPhysicalAddress, hookContext);
    if(returnHandler)
    {
        SUCCESS_OR_CLEANUP(heap->allocate(heap, sizeof(HOOK_CONTEXT), &hookContext));
        hookContext->handler = returnHandler;
        hookContext->virtualAddress = CALC_RETURN_HOOK_ADDR(guestVirtualAddress);
        hookContext->relatedConfig = configContext;
        MapSet(&ext->addressToContext, CALC_RETURN_HOOK_ADDR(guestPhysicalAddress), hookContext);
    }
    // First save the current instruction (must be sent to KPP module)
    SUCCESS_OR_CLEANUP(WinMmCopyGuestMemory(hookedInstruction, guestVirtualAddress, instructionLength));
    // Save it in the config context
    HwCopyMemory(configContext->hookedInstruction, hookedInstruction, instructionLength);
    // Build the hook instruction ((INT3)(INT3-OPTIONAL)(NOP)(NOP)(NOP)(NOP)...)
    hookInstruction[0] = INT3_OPCODE; hookInstruction[1] = INT3_OPCODE;
    HwSetMemory(hookInstruction + 2, NOP_OPCODE, instructionLength - 2);
    // Inject the hooked instruction to the guest and print current stored instruction at address
    Print("Injecting a hook instruction of length %d to (GV) %8. Current instruction is: %.b\n", 
            instructionLength,
            guestVirtualAddress,
            instructionLength,
            WinMmTranslateGuestPhysicalToHostVirtual(guestPhysicalAddress)
            );
    SUCCESS_OR_CLEANUP(WinMmCopyMemoryToGuest(guestVirtualAddress, hookInstruction, instructionLength));
    // Send the hooked instruction to KPP modules
    SUCCESS_OR_CLEANUP(KppAddNewEntry(guestPhysicalAddress, instructionLength, hookedInstruction));
cleanup:
    if(status && hookContext)
        heap->deallocate(heap, hookContext);
    return status;
}

STATUS HookingRemoveHook(IN PCHAR name)
{
    static PMODULE module;
    PHOOKING_MODULE_EXTENSION extension;
    PCONFIG_HOOK_CONTEXT configContext;
    PHOOK_CONTEXT hookContext;
    PSYSCALL_CONFIG_HOOK_CONTEXT syscallContext;
    PLIST_ENTRY listEntry;
    PHEAP heap;
    BOOL found;
    QWORD nameLength, physicalAddress;
    BYTE_PTR virtualAddress;
    STATUS status = STATUS_SUCCESS;

    if(!module)
        MdlGetModuleByName(&module, HOOKING_MODULE_NAME);
    heap = &VmmGetVmmStruct()->currentCPU->sharedData->heap;
    extension = module->moduleExtension;
    nameLength = StringLength(name);
    // Find the associated instruction length from the context
    found = FALSE;
    listEntry = extension->hookConfig.head;
    // Calculate name only once
    nameLength = StringLength(name);
    while(listEntry)
    {
        configContext = (PCONFIG_HOOK_CONTEXT)listEntry->data;
        if(!HwCompareMemory(configContext->name, name, nameLength))
        {
            found = TRUE;
            break;
        }
        listEntry = listEntry->next;
    }
    if(!found)
        return STATUS_UNKNOWN_HOOK_ADDRESS;
    // Convert to virtual
    virtualAddress = WinMmTranslateGuestPhysicalToHostVirtual(configContext->guestPhysicalAddress);
    // Copy the original instruction to guest
    Print("Copying back instruction: %.b to guest, at %8\n", configContext->instructionLength, 
        configContext->hookedInstruction, configContext->guestPhysicalAddress);
    HwCopyMemory(virtualAddress, configContext->hookedInstruction, configContext->instructionLength);
    // Remove hook context from map
    hookContext = MapGet(&extension->addressToContext, configContext->guestPhysicalAddress);
    Print("Removing hook context from map\n");
    MapRemove(&extension->addressToContext, configContext->guestPhysicalAddress);
    if(hookContext->additionalData)
    {
        Print("Found additional data for context, at: %8. Deallocating...\n", hookContext->additionalData);
        heap->deallocate(heap, hookContext->additionalData);
    }
    Print("Deallocating hook context\n");
    heap->deallocate(heap, hookContext);
    // Remove hook context for return event (if exists)
    hookContext = MapGet(&extension->addressToContext, CALC_RETURN_HOOK_ADDR(
        configContext->guestPhysicalAddress));
    if(hookContext != MAP_KEY_NOT_FOUND)
    {
        Print("Found return hook data, releasing all resources\n");
        MapRemove(&extension->addressToContext, CALC_RETURN_HOOK_ADDR(configContext->guestPhysicalAddress));
        heap->deallocate(heap, hookContext);
    }
    // Remove the hook from KPP's map
    Print("Removing entry from KPP's DB\n");
    SUCCESS_OR_CLEANUP(KppRemoveEntry(configContext->guestPhysicalAddress));
    Print("Successfully removed all hook data\n");
cleanup:
    return status;
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
    QWORD syscallId, guestPhysical, ripPhysicalAddress;
    PHOOKING_MODULE_EXTENSION ext;
    PHOOK_CONTEXT hookContext;
    STATUS status = STATUS_SUCCESS;

    vector = vmread(VM_EXIT_INTR_INFO) & 0xff;
    if(vector != INT_BREAKPOINT)
        return STATUS_VM_EXIT_NOT_HANDLED;
    
    ext = module->moduleExtension;
    SUCCESS_OR_CLEANUP(WinMmTranslateGuestVirtualToGuestPhysical(data->guestRegisters.rip, &guestPhysical));
    ripPhysicalAddress = WinMmTranslateGuestPhysicalToPhysicalAddress(guestPhysical);
    if((hookContext = MapGet(&ext->addressToContext, ripPhysicalAddress)) != MAP_KEY_NOT_FOUND)
        return hookContext->handler(hookContext);
    else
        VmmInjectGuestInterrupt(INT_BREAKPOINT, 0);
cleanup:
    return status;
}

REGISTER_MODULE(HookingModuleInitializeAllCores, HookingModuleInitializeSingleCore, hooking);