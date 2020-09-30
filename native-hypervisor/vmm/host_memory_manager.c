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
    return address + VIRTUAL_ADDRESS_OFFET;
}

QWORD InitializeHypervisorPaging(IN PSINGLE_CPU_DATA cpuData)
{
    cpuData->pageMapLevel4s[0] = VirtualToPhysical(cpuData->pageDirectoryPointerTables) | PAGE_PRESENT | PAGE_RW;
    cpuData->pageMapLevel4s[200] = VirtualToPhysical(cpuData->pageDirectoryPointerTables) | PAGE_PRESENT | PAGE_RW;
    for(QWORD i = 0; i < COMPUTER_MEM_SIZE; i++)
        cpuData->pageDirectoryPointerTables[i] = 
            VirtualToPhysical(&(cpuData->pageDirectories[i * ARRAY_PAGE_SIZE])) | PAGE_PRESENT | PAGE_RW;
    
    QWORD physicalAddress = 0;
    for(QWORD i = 0; i < COMPUTER_MEM_SIZE * ARRAY_PAGE_SIZE; 
        i++, physicalAddress += LARGE_PAGE_SIZE)
        cpuData->pageDirectories[i] = physicalAddress | PAGE_LARGE_PAGE | PAGE_RW | PAGE_PRESENT;
    return VirtualToPhysical(cpuData->pageMapLevel4s);
}

QWORD InitializeExtendedPageTable(IN PSINGLE_CPU_DATA cpuData)
{
    EXTENDED_PAGE_TABLE_POINTER eptp;
    eptp.bitFields.dirtyAndAceessEnabled = 1;
    eptp.bitFields.memoryType = 6; // cachable
    eptp.bitFields.pageWalkLength = 3; // pml4, pdpt, pd
    eptp.bitFields.pml4Address = (VirtualToPhysical(cpuData->eptPageMapLevel4s) >> 12);
    eptp.bitFields.reserved = 0;
    eptp.bitFields.reserved1 = 0;

    PEPT_PML4E_PDPTE_PDE pml4p = cpuData->eptPageMapLevel4s;
	pml4p->bitFields.accessed = 0;
	pml4p->bitFields.execute = 1;
	pml4p->bitFields.executeForUserMode = 1;
	pml4p->bitFields.ignored = 0;
	pml4p->bitFields.ignored1 = 0;
	pml4p->bitFields.ignored2 = 0;
	pml4p->bitFields.physicalAddress = (VirtualToPhysical(cpuData->pageDirectoryPointerTables) >> 12);
	pml4p->bitFields.read = 1;
	pml4p->bitFields.reserved = 0;
	pml4p->bitFields.reserved1 = 0;
	pml4p->bitFields.write = 1;

    for(QWORD i = 0; i < COMPUTER_MEM_SIZE; i++)
    {
        PEPT_PML4E_PDPTE_PDE pdptp = &(cpuData->eptPageDirectoryPointerTables[i]);
        pdptp->bitFields.accessed = 0;
        pdptp->bitFields.execute = 1;
        pdptp->bitFields.executeForUserMode = 1;
        pdptp->bitFields.ignored = 0;
        pdptp->bitFields.ignored1 = 0;
        pdptp->bitFields.ignored2 = 0;
        pdptp->bitFields.physicalAddress = (VirtualToPhysical(&(cpuData->eptPageDirectories[i * ARRAY_PAGE_SIZE])) >> 12);
        pdptp->bitFields.read = 1;
        pdptp->bitFields.reserved = 0;
        pdptp->bitFields.reserved1 = 0;
        pdptp->bitFields.write = 1;
    }

    for(QWORD i = 0; i < COMPUTER_MEM_SIZE * ARRAY_PAGE_SIZE; i++)
    {
        PEPT_PML4E_PDPTE_PDE pdp = &(cpuData->eptPageDirectories[i]);
        pdp->bitFields.accessed = 0;
        pdp->bitFields.execute = 1;
        pdp->bitFields.executeForUserMode = 1;
        pdp->bitFields.ignored = 0;
        pdp->bitFields.ignored1 = 0;
        pdp->bitFields.ignored2 = 0;
        pdp->bitFields.physicalAddress = (VirtualToPhysical(&(cpuData->eptPageDirectories[i * ARRAY_PAGE_SIZE])) >> 12);
        pdp->bitFields.read = 1;
        pdp->bitFields.reserved = 0;
        pdp->bitFields.reserved1 = 0;
        pdp->bitFields.write = 1;
    }

    for(QWORD i = 0; i < COMPUTER_MEM_SIZE * ARRAY_PAGE_SIZE * ARRAY_PAGE_SIZE; i++)
    {
        PEPT_PTE ptp = &(cpuData->eptPageTables[i]);
		ptp->bitFields.accessedFlag = 0;
		ptp->bitFields.dirtyFlag = 0;
		ptp->bitFields.eptMemoryType = 6;
		ptp->bitFields.execute = 1;
		ptp->bitFields.executeForUserMode = 1;
		ptp->bitFields.ignorePAT = 0;
		ptp->bitFields.physicalAddress = i;
		ptp->bitFields.read = 1;
		ptp->bitFields.suppressVE = 0;
		ptp->bitFields.write = 1;
    }
    
    return eptp.value;
}