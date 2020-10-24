#include <vmm/vmm.h>
#include <debug.h>
#include <intrinsics.h>
#include <vmm/msr.h>
#include <vmm/memory_manager.h>
#include <vmm/vm_operations.h>
#include <vmm/vmcs.h>
#include <vmm/control_fields.h>
#include <x86_64.h>
#include <vmm/exit_reasons.h>
#include <win_kernel/memory_manager.h>
#include <bios/apic.h>
#include <vmx_modules/default_module.h>
#include <vmx_modules/syscalls_module.h>

VOID InitializeSingleHypervisor(IN PVOID data)
{
    PSINGLE_CPU_DATA cpuData = (PSINGLE_CPU_DATA)data;
    Print("Initializing hypervisor on core #%d\n", cpuData->coreIdentifier);

    __writecr0((__readcr0() | CR0_NE_ENABLED | __readmsr(MSR_IA32_VMX_CR0_FIXED0)) 
                 & __readmsr(MSR_IA32_VMX_CR0_FIXED1));
    __writecr4((__readcr4() | CR4_VMX_ENABLED | __readmsr(MSR_IA32_VMX_CR4_FIXED0))
                 & __readmsr(MSR_IA32_VMX_CR4_FIXED1));
    // Enable VMXON in SMX and non-SMX. Also set the lock bit
    __writemsr(MSR_IA32_FEATURE_CONTROL, __readmsr(MSR_IA32_FEATURE_CONTROL) | 7);
    // Prepare VMXON and VMCS regions
    *(DWORD_PTR)(cpuData->vmcs) = (DWORD)__readmsr(MSR_IA32_VMX_BASIC);
    *(DWORD_PTR)(cpuData->vmxon) = (DWORD)__readmsr(MSR_IA32_VMX_BASIC);
    // Enter VMX operation
    ASSERT(__vmxon(VirtualToPhysical(cpuData->vmxon)) == STATUS_SUCCESS);
    // VMCS status will now turn to cleared
    ASSERT(__vmclear(VirtualToPhysical(cpuData->vmcs)) == STATUS_SUCCESS);
    // Current & active
    ASSERT(__vmptrld(VirtualToPhysical(cpuData->vmcs)) == STATUS_SUCCESS);  
    cpuData->gdt[0] = 0x0ULL;
    cpuData->gdt[1] = 0x00a09b0000000000ULL;
    cpuData->gdt[2] = 0x00cf93000000ffffULL;
    __writemsr(MSR_IA32_DEBUGCTL, __readmsr(MSR_IA32_DEBUGCTL) & 0xffc3); // see Intel SDM, 26.3.1.1, 4th point
	__writedr7(__readdr7() & 0xffffffff);
    // Initialize guest area
    __vmwrite(GUEST_CR0, __readcr0());
    __vmwrite(GUEST_CR3, __readcr3());
    __vmwrite(GUEST_CR4, __readcr4());
    GDT gdt;
    IDT idt;
    GetGDT(&gdt);
    GetIDT(&idt);
    __vmwrite(GUEST_GDTR_BASE, gdt.address);
    __vmwrite(GUEST_GDTR_LIMIT, gdt.limit);
    __vmwrite(GUEST_IDTR_BASE, idt.address);
    __vmwrite(GUEST_IDTR_LIMIT, idt.limit);
    __vmwrite(GUEST_LDTR_BASE, 0);
    __vmwrite(GUEST_LDTR_LIMIT, 0xff);
    __vmwrite(GUEST_LDTR_AR_BYTES, VMCS_SELECTOR_UNUSABLE); // <--- so much time was spent on this line...
    // To understand the AR fields, see section 24.4.1 on Intel DSM
    /// Note for the future: Try to clear reseved fields (in AR VMCS fields) if vmlaunch is failing
    // CS related data
    __vmwrite(GUEST_CS_SELECTOR, GetCS() & 0xf8);
    __vmwrite(GUEST_CS_BASE, 0ULL);
    __vmwrite(GUEST_CS_LIMIT, 0xffffffff);
    __vmwrite(GUEST_CS_AR_BYTES, 0xa09b);
    // DS related data
    __vmwrite(GUEST_DS_SELECTOR, GetDS() & 0xf8);
    __vmwrite(GUEST_DS_BASE, 0ULL);
    __vmwrite(GUEST_DS_LIMIT, 0xffffffff);
    __vmwrite(GUEST_DS_AR_BYTES, 0xc093);
    // SS related data
    __vmwrite(GUEST_SS_SELECTOR, GetSS() & 0xf8);
    __vmwrite(GUEST_SS_BASE, 0ULL);
    __vmwrite(GUEST_SS_LIMIT, 0xffffffff);
    __vmwrite(GUEST_SS_AR_BYTES, 0xc093);
    // ES related data
    __vmwrite(GUEST_ES_SELECTOR, GetES() & 0xf8);
    __vmwrite(GUEST_ES_BASE, 0ULL);
    __vmwrite(GUEST_ES_LIMIT, 0xffffffff);
    __vmwrite(GUEST_ES_AR_BYTES, 0xc093);
    // GS related data
    __vmwrite(GUEST_GS_SELECTOR, GetGS() & 0xf8);
    __vmwrite(GUEST_GS_BASE, 0ULL);
    __vmwrite(GUEST_GS_LIMIT, 0xffffffff);
    __vmwrite(GUEST_GS_AR_BYTES, 0xc093);
    // FS related data
    __vmwrite(GUEST_FS_SELECTOR, GetFS() & 0xf8);
    __vmwrite(GUEST_FS_BASE, 0ULL);
    __vmwrite(GUEST_FS_LIMIT, 0xffffffff);
    __vmwrite(GUEST_FS_AR_BYTES, 0xc093);
    // TR related data
    __vmwrite(GUEST_TR_SELECTOR, GetDS() & 0xf8);
    __vmwrite(GUEST_TR_BASE, 0ULL);
    __vmwrite(GUEST_TR_LIMIT, 0xffffffff);
    __vmwrite(GUEST_TR_AR_BYTES, 0xc08b);
    __vmwrite(GUEST_RIP, VmmToVm);
    __vmwrite(GUEST_RSP, 0); // Will be handled before vmlaunch is called, see x86_64.asm
    __vmwrite(GUEST_EFER, __readmsr(MSR_IA32_EFER));
    __vmwrite(GUEST_RFLAGS, __readflags() & 0x1fffd7);
    __vmwrite(GUEST_IA32_DEBUGCTL, __readmsr(MSR_IA32_DEBUGCTL) & 0xffffffff);
    __vmwrite(GUEST_IA32_DEBUGCTL_HIGH, __readmsr(MSR_IA32_DEBUGCTL) >> 32);
    __vmwrite(GUEST_INTERRUPTIBILITY_INFO, 0);
    __vmwrite(GUEST_ACTIVITY_STATE, CPU_STATE_ACTIVE);
    __vmwrite(GUEST_SYSENTER_EIP, 0xffff);
    __vmwrite(GUEST_SYSENTER_ESP, 0xffff);
    __vmwrite(GUEST_SYSENTER_CS, 8);
    __vmwrite(GUEST_DR7, __readdr7());
    __vmwrite(CR0_GUEST_HOST_MASK, 0);
    __vmwrite(CR4_GUEST_HOST_MASK, 0);
    
    // Initialize host area
    __vmwrite(HOST_CR0, __readcr0());
    __vmwrite(HOST_CR3, InitializeHypervisorPaging(cpuData));
    __vmwrite(HOST_CR4, __readcr4() | CR4_OSXSAVE | (1 << 9));
    __vmwrite(HOST_RIP, HandleVmExit);
    __vmwrite(HOST_RSP, cpuData->stack + sizeof(cpuData->stack)); // from high addresses to lower
    __vmwrite(HOST_FS_BASE, cpuData->sharedData->currentState[cpuData->coreIdentifier]);
    __vmwrite(HOST_GS_BASE, 0);
    __vmwrite(HOST_CS_SELECTOR, HYPERVISOR_CS_SELECTOR);
    __vmwrite(HOST_DS_SELECTOR, HYPERVISOR_DS_SELECTOR);
    __vmwrite(HOST_ES_SELECTOR, HYPERVISOR_DS_SELECTOR);
    __vmwrite(HOST_SS_SELECTOR, HYPERVISOR_DS_SELECTOR);
    __vmwrite(HOST_GS_SELECTOR, HYPERVISOR_DS_SELECTOR);
    __vmwrite(HOST_FS_SELECTOR, HYPERVISOR_DS_SELECTOR); 
    __vmwrite(HOST_TR_SELECTOR, HYPERVISOR_DS_SELECTOR);
    __vmwrite(HOST_TR_BASE, HYPERVISOR_DS_SELECTOR);
    __vmwrite(HOST_EFER, __readmsr(MSR_IA32_EFER));
    __vmwrite(HOST_SYSENTER_CS, 0xff);
    __vmwrite(HOST_SYSENTER_EIP, 0xffffffff);
    __vmwrite(HOST_SYSENTER_ESP, 0xffffffff);
    __vmwrite(HOST_GDTR_BASE, cpuData->gdt);

    // General
    __vmwrite(VMCS_LINK_POINTER, -1ULL);
    __vmwrite(TSC_OFFSET, 0);
    __vmwrite(TSC_OFFSET_HIGH, 0);
    __vmwrite(PAGE_FAULT_ERROR_CODE_MASK, 0);
    __vmwrite(PAGE_FAULT_ERROR_CODE_MATCH, 0);
    __vmwrite(VM_EXIT_MSR_STORE_COUNT, 0);
    __vmwrite(VM_EXIT_MSR_LOAD_COUNT, 0);
    __vmwrite(VM_ENTRY_MSR_LOAD_COUNT, 0);
    __vmwrite(VM_ENTRY_INTR_INFO, 0);
    __vmwrite(CPU_BASED_VM_EXEC_CONTROL, AdjustControls(CPU_BASED_ACTIVATE_MSR_BITMAP | CPU_BASED_ACTIVATE_SECONDARY_CONTROLS, MSR_IA32_VMX_PROCBASED_CTLS));
    __vmwrite(SECONDARY_VM_EXEC_CONTROL, AdjustControls(CPU_BASED_CTL2_ENABLE_INVPCID | CPU_BASED_CTL2_RDTSCP  | CPU_BASED_CTL2_ENABLE_EPT | CPU_BASED_CTL2_UNRESTRICTED_GUEST, MSR_IA32_VMX_PROCBASED_CTLS2));
    __vmwrite(PIN_BASED_VM_EXEC_CONTROL, AdjustControls(0, MSR_IA32_VMX_PINBASED_CTLS));
    __vmwrite(VM_EXIT_CONTROLS, AdjustControls(VM_EXIT_SAVE_EFER | VM_EXIT_LOAD_EFER | VM_EXIT_IA32E_MODE, MSR_IA32_VMX_EXIT_CTLS));
    __vmwrite(VM_ENTRY_CONTROLS, AdjustControls(VM_ENTRY_LOAD_EFER | VM_ENTRY_IA32E_MODE, MSR_IA32_VMX_ENTRY_CTLS));
    __vmwrite(EPT_POINTER, InitializeExtendedPageTable(cpuData));
    __vmwrite(MSR_BITMAP, VirtualToPhysical(cpuData->msrBitmaps));
    __vmwrite(VIRTUAL_PROCESSOR_ID, 1);
    
    if(SetupCompleteBackToGuestState() != STATUS_SUCCESS)
    {
        // Should never arrive here
        Print("FLAGS: %8, instruction error: %8\n", __readflags(), vmread(VM_INSTRUCTION_ERROR));
        ASSERT(FALSE);
    }
    Print("Initialization completed on core #%d\n", cpuData->coreIdentifier);
}

DWORD AdjustControls(IN DWORD control, IN QWORD msr)
{
	QWORD msrValue = __readmsr(msr);
	control &= (msrValue >> 32);            // force 0 if the corresponding MSR requires it
	control |= (msrValue & 0xffffffffULL); // force 1 if the corresponding MSR requires it
	return control;
}

VOID HandleVmExitEx()
{
    QWORD exitReason = vmread(VM_EXIT_REASON);
    QWORD exitQualification = vmread(EXIT_QUALIFICATION);
    if(exitReason & VM_ENTRY_FAILURE_MASK)
    {
        Print("VM-Entry failure occured. Exit qualification: %d\n", exitQualification);
        goto DefaultHandler;
    }
    exitReason &= 0xffff; // 0..15, Intel SDM 26.7
    PCURRENT_GUEST_STATE data = GetVMMStruct();
    PSHARED_CPU_DATA shared = data->currentCPU->sharedData;
    for(QWORD i = 0; i < shared->modulesCount; i++)
        if(shared->modules[i]->isHandledOnVmExit[exitReason])
            if(shared->modules[i]->vmExitHandlers[exitReason](data) == STATUS_SUCCESS)
                return;
DefaultHandler:
    if(shared->defaultModule.isHandledOnVmExit[exitReason])
    {
        STATUS handleStatus;
        if(handleStatus = shared->defaultModule.vmExitHandlers[exitReason](data))
        {
            Print("Error during handling vm-exit. Exit reaon: %d, Error code: %d", exitReason, handleStatus);
            ASSERT(FALSE);
        }
    }
    else
    {
        Print("Unhandled vm-exit occured. Exit reason is: %d\n", exitReason);
        ASSERT(FALSE);
    }
}

PCURRENT_GUEST_STATE GetVMMStruct()
{
    return (PCURRENT_GUEST_STATE)vmread(HOST_FS_BASE);
}

STATUS SetupHypervisorCodeProtection(IN PSHARED_CPU_DATA data, IN QWORD codeBase, IN QWORD codeLength)
{
    if(codeBase % PAGE_SIZE)
        return STATUS_MEMORY_NOT_ALIGNED;
    PrintDebugLevelDebug("Protecting code base %8, length %8 with access 0...\n", 
        codeBase, codeLength);
    PrintDebugLevelDebug("Protecting hypervisor base %8, length %8 with access 0...\n", 
        data->physicalHypervisorBase, data->hypervisorBaseSize);
    QWORD codeSizeInPages = ALIGN_UP(codeLength, PAGE_SIZE) / PAGE_SIZE, 
        hypervisorBaseSizeInPages = data->hypervisorBaseSize / PAGE_SIZE;
    for(QWORD i = 0; i < data->numberOfCores; i++)
    {
        ASSERT(UpdateEptAccessPolicy(data->cpuData[i], data->physicalCodeBase, data->codeBaseSize,
            0) == STATUS_SUCCESS);
        ASSERT(UpdateEptAccessPolicy(data->cpuData[i], data->physicalHypervisorBase,
            data->hypervisorBaseSize, 0) == STATUS_SUCCESS);
    }
    PrintDebugLevelDebug("Done mapping\n");
    return STATUS_SUCCESS;
}

BOOL CheckAccessToHiddenBase(IN PSHARED_CPU_DATA data, IN QWORD accessedAddress)
{
    return (accessedAddress >= data->physicalHypervisorBase) && (accessedAddress <= data->physicalHypervisorBase 
        + data->hypervisorBaseSize) ? STATUS_ACCESS_TO_HIDDEN_BASE : STATUS_SUCCESS;
}

STATUS UpdateEptAccessPolicy(IN PSINGLE_CPU_DATA data, IN QWORD base, IN QWORD length, IN QWORD access)
{
    if(base % PAGE_SIZE || length % PAGE_SIZE)
    {
        PrintDebugLevelDebug("Could not update EPT policy. Memory not aligned: %8 %8\n", base, length);
        return STATUS_MEMORY_NOT_ALIGNED;
    }
    if(access > 7)
    {
        PrintDebugLevelDebug("Could not update EPT policy. Invalid access rights: %d\n", access);
        return STATUS_INVALID_ACCESS_RIGHTS;
    }
    PrintDebugLevelDebug("Updating EPT policy. Guest physical: %8, length: %8, access: %d...\n", 
        base, length, access);
    QWORD lengthInPages = length / PAGE_SIZE;
    for(QWORD i = 0; i < lengthInPages; i++)
        data->eptPageTables[base / PAGE_SIZE + i] = data->eptPageTables[base / PAGE_SIZE + i] 
            & EPT_ACCESS_MASK | access;
    PrintDebugLevelDebug("EPT policy updated\n");
    return STATUS_SUCCESS;
}

STATUS UpdateMsrAccessPolicy(IN PSHARED_CPU_DATA sharedData, IN QWORD msrNumber, IN BOOL read, IN BOOL write)
{
    BYTE range;
    if(!IsMsrValid(msrNumber, &range))
        return STATUS_INVALID_MSR;
    QWORD msrReadIdx = (range == MSR_RANGE_FIRST) ? msrNumber : msrNumber - 0xc0000000 + 1024, 
        msrWriteIdx = (range == MSR_RANGE_FIRST) ? msrNumber + 2048 : msrNumber - 0xc0000000 + 3072;
    for(QWORD i = 0; i < sharedData->numberOfCores; i++)
    {
        BYTE_PTR bitmap = sharedData->cpuData[i]->msrBitmaps;
        if(read)
            bitmap[msrReadIdx / 8] |= (1 << (msrReadIdx % 8));
        else
            bitmap[msrReadIdx / 8] &= ~(1 << (msrReadIdx % 8));
        if(write)
             bitmap[msrWriteIdx / 8] |= (1 << (msrReadIdx % 8));
        else
            bitmap[msrWriteIdx / 8] &= ~(1 << (msrReadIdx % 8));
    }
}

STATUS SetupE820Hook(IN PSHARED_CPU_DATA sharedData)
{
    DWORD_PTR ivtAddress = PhysicalToVirtual(0);
    QWORD segment = (ivtAddress[0x15] >> 16) & 0xffffULL;
    QWORD offset = ivtAddress[0x15] & 0xffffULL;
    BYTE vmcall[] = { 0x0f, 0x01, 0xc1 };
    CopyMemory(E820_VMCALL_GATE, vmcall, 3);
    ivtAddress[0x15] = E820_VMCALL_GATE;
    sharedData->int15Offset = offset;
    sharedData->int15Segment = (segment << 4);
    if(ivtAddress[0x15] != E820_VMCALL_GATE)
        return STATUS_E820_NOT_HOOKED;
    return STATUS_SUCCESS;
}

VOID RegisterAllModules(IN PSHARED_CPU_DATA sharedData)
{
    // Init modules data
    sharedData->modules = NULL;
    sharedData->modulesCount = 0;
    InitModule(sharedData, &sharedData->defaultModule, NULL);
    // Default module
    SetModuleName(sharedData, &sharedData->defaultModule, "Default Module");
    RegisterVmExitHandler(&sharedData->defaultModule, EXIT_REASON_MSR_READ, HandleMsrRead);
    RegisterVmExitHandler(&sharedData->defaultModule, EXIT_REASON_MSR_WRITE, HandleMsrWrite);
    RegisterVmExitHandler(&sharedData->defaultModule, EXIT_REASON_INVALID_GUEST_STATE, HandleInvalidGuestState);
    RegisterVmExitHandler(&sharedData->defaultModule, EXIT_REASON_XSETBV, EmulateXSETBV);
    RegisterVmExitHandler(&sharedData->defaultModule, EXIT_REASON_CPUID, HandleCpuId);
    RegisterVmExitHandler(&sharedData->defaultModule, EXIT_REASON_CR_ACCESS, HandleCrAccess);
    RegisterVmExitHandler(&sharedData->defaultModule, EXIT_REASON_EPT_VIOLATION, HandleEptViolation);
    RegisterVmExitHandler(&sharedData->defaultModule, EXIT_REASON_VMCALL, HandleVmCall);
    RegisterVmExitHandler(&sharedData->defaultModule, EXIT_REASON_MSR_LOADING, HandleInvalidMsrLoading);
    RegisterVmExitHandler(&sharedData->defaultModule, EXIT_REASON_MCE_DURING_VMENTRY, HandleMachineCheckFailure);
    RegisterVmExitHandler(&sharedData->defaultModule, EXIT_REASON_TRIPLE_FAULT, HandleTripleFault);
    RegisterVmExitHandler(&sharedData->defaultModule, EXIT_REASON_INIT, HandleApicInit);
    RegisterVmExitHandler(&sharedData->defaultModule, EXIT_REASON_SIPI, HandleApicSipi);
    Print("Successfully registered defualt module\n");
    // Dynamic modules
    // KPP Module
    PMODULE syscallsModule;
    sharedData->heap.allocate(&sharedData->heap, sizeof(MODULE), &syscallsModule);
    InitModule(sharedData, syscallsModule, SyscallsModuleInitialize);
    SetModuleName(sharedData, syscallsModule, "Syscalls Module");
    RegisterVmExitHandler(syscallsModule, EXIT_REASON_MSR_WRITE, SyscallsHandleMsrWrite);
    RegisterModule(sharedData, syscallsModule);
    Print("Successfully registered syscalls module\n");
}