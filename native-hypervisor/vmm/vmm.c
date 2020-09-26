#include <vmm/vmm.h>
#include <debug.h>
#include <intrinsics.h>
#include <vmm/msr.h>
#include <vmm/memory_manager.h>
#include <vmm/vmcs.h>
#include <x86_64.h>

VOID InitializeSingleHypervisor(IN PVOID data)
{
    PSINGLE_CPU_DATA cpuData = (PSINGLE_CPU_DATA)data;
    Print("Initializing hypervisor on core #%d\n", cpuData->coreIdentifier);

    __writecr0((__readcr0() | __readmsr(MSR_IA32_VMX_CR0_FIXED0)) & __readmsr(MSR_IA32_VMX_CR0_FIXED1));
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
    __vmwrite(GUEST_CS_SELECTOR, HYPERVISOR_CS_SELECTOR);
    __vmwrite(GUEST_DS_SELECTOR, HYPERVISOR_DS_SELECTOR);
    __vmwrite(GUEST_SS_SELECTOR, HYPERVISOR_DS_SELECTOR);
    __vmwrite(GUEST_RIP, VmmToVm);
    __vmwrite(GUEST_RSP, 0); // Will be handled before vmlaunch is called
    __vmwrite(GUEST_EFER, __readmsr(MSR_IA32_EFER));

    // Initialize host area
    __vmwrite(HOST_CR0, __readcr0());
    __vmwrite(HOST_CR3, InitializeHypervisorPaging(cpuData));
    __vmwrite(HOST_CR4, __readcr4());
    __vmwrite(HOST_RIP, HandleVmExit);
    __vmwrite(HOST_RSP, cpuData->stack + sizeof(cpuData->stack)); // from high addresses to lower
    __vmwrite(HOST_FS_BASE, cpuData->sharedData->currentState[cpuData->coreIdentifier]);
    __vmwrite(HOST_GS_BASE, 0);
    __vmwrite(HOST_EFER, __readmsr(MSR_IA32_EFER));
    __vmwrite(HOST_SYSENTER_CS, 0);
    __vmwrite(HOST_SYSENTER_EIP, 0);
    __vmwrite(HOST_SYSENTER_ESP, 0);
    
    
}