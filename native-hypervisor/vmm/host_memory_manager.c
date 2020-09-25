#include <vmm/memory_manager.h>
#include <vmm/vmm.h>

VOID SetupVirtualAddress(IN QWORD pml4BaseAddress)
{
    ((QWORD_PTR)pml4BaseAddress)[200] = ((QWORD_PTR)pml4BaseAddress)[0];
}

QWORD VirtualToPhysical(IN QWORD address)
{
    return address - VIRTUAL_ADDRESS_OFFET;
}

QWORD PhysicalToVirtual(IN QWORD address)
{
    return address | VIRTUAL_ADDRESS_OFFET;
}

VOID InitializeHypervisorPaging(IN PSINGLE_CPU_DATA cpuData)
{
    cpuData->pageMapLevel4s[0] = cpuData->pageDirectoryPointerTables;
    cpuData->pageMapLevel4s[200] = cpuData->pageDirectoryPointerTables;
    for(QWORD i = 0; i < COMPUTER_MEM_SIZE; i++)
        cpuData->pageDirectoryPointerTables[i] = cpuData->pageDirectories[i * PAGE_SIZE]
                                                    | PAGE_PRESENT | PAGE_RW;
    QWORD physicalAddress = 0;
    for(QWORD i = 0; i < COMPUTER_MEM_SIZE * ARRAY_PAGE_SIZE; 
        i++, physicalAddress += LARGE_PAGE_SIZE)
        cpuData->pageDirectories[i] = physicalAddress | PAGE_LARGE_PAGE | PAGE_RW | PAGE_PRESENT;
    return VirtualToPhysical(cpuData->pageMapLevel4s);
}