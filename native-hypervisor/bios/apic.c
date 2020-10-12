#include <bios/bios_os_loader.h>
#include <debug.h>
#include <intrinsics.h>
#include <x86_64.h>
#include <vmm/msr.h>
#include <bios/apic.h>
#include <vmm/vmm.h>

STATUS GetCoresData(IN BYTE_PTR apicTable, OUT BYTE_PTR processorsCount, OUT BYTE_PTR processorsIdentifiers)
{
    QWORD tableLength = *(DWORD_PTR)(apicTable + RSDT_LENGTH_OFFSET);
    *processorsCount = 0;
    for(QWORD offset = 0x2C; offset < tableLength;)
    {
        switch(*(BYTE_PTR)(apicTable + offset))
        {
            case PROCESSOR_LOCAL_APIC:
                processorsIdentifiers[(*processorsCount)++] = *((BYTE_PTR)apicTable + offset + 2);
                break;
            /* reserved for future usage */
        }
        offset += *((BYTE_PTR)apicTable + offset + 1);
    }

    if(!(*processorsCount))
        return STATUS_NO_CORES_FOUND;
    
    return STATUS_SUCCESS;
}

STATUS DetectX2APICAvailability()
{
    QWORD tmp, ecx;
    __cpuid(1, 0, &tmp, &tmp, &ecx, &tmp);
    return (ecx & CPUID_2XAPIC_AVAILABLE) ? STATUS_SUCCESS : STATUS_2XAPIC_NOT_AVAILABLE;
}

VOID EnableX2APIC()
{
    ASSERT(DetectX2APICAvailability() == STATUS_SUCCESS);
    __writemsr(MSR_IA32_APIC_BASE, __readmsr(MSR_IA32_APIC_BASE) | (1 << 10) | (1 << 11));
}

VOID EnableAPIC()
{
    __writemsr(MSR_IA32_APIC_BASE, __readmsr(MSR_IA32_APIC_BASE) | (1 << 11));
}

VOID X2APICIssueIPI(IN QWORD destenation, IN QWORD vector, IN QWORD deliveryMode, IN QWORD lvl)
{
    QWORD ipi = (destenation << APIC_DESTINAION_BIT_OFFSET) | deliveryMode | vector | lvl;
    __writemsr(MSR_IA32_X2APIC_ICR, ipi);
}

VOID APICGetBaseAddress()
{
    return __readmsr(MSR_IA32_APIC_BASE) & 0xffffff000;
}

STATUS ActivateHypervisorOnProcessor(IN QWORD processorId, IN PSINGLE_CPU_DATA cpuData)
{
    CopyMemory(APIC_FUNC_ADDRESS, ApicStart, ApicEnd - ApicStart);
    X2APICIssueIPI(processorId, 0, APIC_INIT_INTERRUPT, APIC_LEVEL_DEASSERT);
    Sleep(50);
    *(QWORD_PTR)CPU_DATA_ADDRESS = cpuData;
    *(QWORD_PTR)(CPU_DATA_ADDRESS + sizeof(QWORD)) = InitializeSingleHypervisor;
    *(BYTE_PTR)SEMAPHORE_LOCATION = 0;
    X2APICIssueIPI(processorId, APIC_FUNC_ADDRESS / PAGE_SIZE, APIC_SIPI_INTERRUPT, APIC_LEVEL_ASSERT);
    Sleep(50);
    for(QWORD i = 0; i < 0x11ffffffffffffffULL && (*(BYTE_PTR)SEMAPHORE_LOCATION == 0); i++);
    return *(BYTE_PTR)SEMAPHORE_LOCATION == 0 ? STATUS_APIC_SIPI_FAILED : STATUS_SUCCESS;
}