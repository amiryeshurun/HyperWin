#include <vmm/vmm.h>
#include <debug.h>
#include <intrinsics.h>
#include <vmm/msr.h>
#include <vmm/memory_manager.h>

VOID InitializeSingleHypervisor(IN PVOID data)
{
    PSINGLE_CPU_DATA cpuData = (PSINGLE_CPU_DATA)data;
    Print("Initializing hypervisor on core #%d\n", cpuData->coreIdentifier);

    __writecr0((__readcr0() | __readmsr(MSR_IA32_VMX_CR0_FIXED0)) & __readmsr(MSR_IA32_VMX_CR0_FIXED1));
    __writecr4((__readcr4() | CR4_VMX_ENABLED | __readmsr(MSR_IA32_VMX_CR4_FIXED0))
                 & __readmsr(MSR_IA32_VMX_CR4_FIXED1));
    // Enable VMXON in SMX and non-SMX. Also set the lock bit
    __writemsr(MSR_IA32_FEATURE_CONTROL, __readmsr(MSR_IA32_FEATURE_CONTROL) | 7);
    *(DWORD_PTR)(cpuData->vmcs) = (DWORD)__readmsr(MSR_IA32_VMX_BASIC);
    *(DWORD_PTR)(cpuData->vmxon) = (DWORD)__readmsr(MSR_IA32_VMX_BASIC);
    Print("%8\n", VirtualToPhysical(cpuData->vmxon));
    ASSERT(__vmxon(VirtualToPhysical(cpuData->vmxon)) == 0);
    Print("After vmxon\n");
}