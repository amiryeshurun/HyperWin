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

VOID InitializeSingleHypervisor(IN PVOID data)
{
    PSINGLE_CPU_DATA cpuData = (PSINGLE_CPU_DATA)data;
    Print("Initializing hypervisor on core #%d\n", cpuData->coreIdentifier);

    __writecr0((__readcr0() | CR0_NE_ENABLED  | __readmsr(MSR_IA32_VMX_CR0_FIXED0)) 
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
    __vmwrite(GUEST_LDTR_BASE, 0ULL);
    __vmwrite(GUEST_LDTR_LIMIT, 0xff);
    __vmwrite(GUEST_LDTR_AR_BYTES, VMCS_SELECTOR_UNUSABLE);
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
	__vmwrite(GUEST_ACTIVITY_STATE, 0);
    __vmwrite(GUEST_SYSENTER_EIP, 0xffff);
    __vmwrite(GUEST_SYSENTER_ESP, 0xffff);
    __vmwrite(GUEST_SYSENTER_CS, 8);
    __vmwrite(GUEST_DR7, __readdr7());

    // Initialize host area
    __vmwrite(HOST_CR0, __readcr0());
    __vmwrite(HOST_CR3, InitializeHypervisorPaging(cpuData));
    __vmwrite(HOST_CR4, __readcr4());
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
    __vmwrite(CPU_BASED_VM_EXEC_CONTROL, AdjustControls(CPU_BASED_ACTIVATE_SECONDARY_CONTROLS, MSR_IA32_VMX_PROCBASED_CTLS));
	__vmwrite(SECONDARY_VM_EXEC_CONTROL, AdjustControls(CPU_BASED_CTL2_ENABLE_EPT | CPU_BASED_CTL2_UNRESTRICTED_GUEST, MSR_IA32_VMX_PROCBASED_CTLS2));
    __vmwrite(PIN_BASED_VM_EXEC_CONTROL, AdjustControls(0, MSR_IA32_VMX_PINBASED_CTLS));
	__vmwrite(VM_EXIT_CONTROLS, AdjustControls(VM_EXIT_IA32E_MODE | VM_EXIT_ACK_INTR_ON_EXIT, MSR_IA32_VMX_EXIT_CTLS));
	__vmwrite(VM_ENTRY_CONTROLS, AdjustControls(VM_ENTRY_IA32E_MODE | VM_ENTRY_LOAD_DEBUG_CTLS, MSR_IA32_VMX_ENTRY_CTLS));
    __vmwrite(EPT_POINTER, InitializeExtendedPageTable(cpuData));
    QWORD flags = SetupCompleteBackToGuestState();
    // Should never arrive here
    Print("FLAGS: %8, instruction error: %8\n", flags, vmread(VM_INSTRUCTION_ERROR));
    ASSERT(FALSE);
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
    if(exitReason & VM_ENTRY_FAILURE_MASK)
        Print("VM-Entry failure occured. Exit qualification: %d\n", vmread(EXIT_QUALIFICATION));
    
    PCURRENT_GUEST_STATE data = GetVMMStruct();
    switch(exitReason & 0xffff) // 0..15, Intel SDM 26.7
    {
        case EXIT_REASON_EPT_VIOLATION:
            Print("EPT Violation occured at: (P)%8, (V)%8\n", 
                vmread(GUEST_PHYSICAL_ADDRESS), data->guestRegisters.rip);
            ASSERT(FALSE);
        case EXIT_REASON_INVALID_GUEST_STATE:
            Print("INVALID GUEST STATE!\n");
            ASSERT(FALSE);
            break;
        case EXIT_REASON_MSR_LOADING:
            Print("INVALID MSR LOADING!\n");
            ASSERT(FALSE);
            break;
        case EXIT_REASON_MCE_DURING_VMENTRY:
            Print("Failure due to machine-check event!\n");
            ASSERT(FALSE);
            break;
        default:
            Print("No match found, exit reason %d\n", exitReason);
            ASSERT(FALSE);
    }
}

PCURRENT_GUEST_STATE GetVMMStruct()
{
    return (PCURRENT_GUEST_STATE)vmread(HOST_FS_BASE);
}