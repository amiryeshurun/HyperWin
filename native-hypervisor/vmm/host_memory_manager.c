#include <vmm/memory_manager.h>
#include <vmm/vmm.h>

VOID VmmSetupVirtualAddress(IN QWORD pml4BaseAddress)
{
    ((QWORD_PTR)pml4BaseAddress)[200] = ((QWORD_PTR)pml4BaseAddress)[0];
}

QWORD VirtualToPhysical(IN BYTE_PTR address)
{
    return address - VIRTUAL_ADDRESS_OFFET;
}

BYTE_PTR PhysicalToVirtual(IN QWORD address)
{
    return address + VIRTUAL_ADDRESS_OFFET;
}

QWORD VmmInitializeHypervisorPaging(IN PSINGLE_CPU_DATA cpuData)
{
    QWORD physicalAddress;

    cpuData->pageMapLevel4s[0] = VirtualToPhysical(cpuData->pageDirectoryPointerTables) | PAGE_PRESENT | PAGE_RW;
    cpuData->pageMapLevel4s[200] = VirtualToPhysical(cpuData->pageDirectoryPointerTables) | PAGE_PRESENT | PAGE_RW;
    for(QWORD i = 0; i < COMPUTER_MEM_SIZE; i++)
        cpuData->pageDirectoryPointerTables[i] = 
            VirtualToPhysical(&(cpuData->pageDirectories[i * ARRAY_PAGE_SIZE])) | PAGE_PRESENT | PAGE_RW;
    
    physicalAddress = 0;
    for(QWORD i = 0; i < COMPUTER_MEM_SIZE * ARRAY_PAGE_SIZE; 
        i++, physicalAddress += LARGE_PAGE_SIZE)
        cpuData->pageDirectories[i] = physicalAddress | PAGE_LARGE_PAGE | PAGE_RW | PAGE_PRESENT;
    return VirtualToPhysical(cpuData->pageMapLevel4s);
}

QWORD VmmInitializeExtendedPageTable(IN PSINGLE_CPU_DATA cpuData)
{
    QWORD eptp, physicalAddress;
    
    eptp = VirtualToPhysical(cpuData->eptPageMapLevel4s) | EPT_PAGE_WALK_LENGTH | EPT_POINTER_WB;
    cpuData->eptPageMapLevel4s[0] = VirtualToPhysical(cpuData->eptPageDirectoryPointerTables) | EPT_RWX;
    for(QWORD i = 0; i < COMPUTER_MEM_SIZE; i++)
        cpuData->eptPageDirectoryPointerTables[i] = VirtualToPhysical(&(cpuData->eptPageDirectories[i * ARRAY_PAGE_SIZE]))
            | EPT_RWX;
    for(QWORD i = 0; i < COMPUTER_MEM_SIZE * ARRAY_PAGE_SIZE; i++)
        cpuData->eptPageDirectories[i] = VirtualToPhysical(&(cpuData->eptPageTables[i * ARRAY_PAGE_SIZE])) | EPT_RWX;
    physicalAddress = 0;
    for(QWORD i = 0; i < COMPUTER_MEM_SIZE * ARRAY_PAGE_SIZE * ARRAY_PAGE_SIZE; i++, physicalAddress += PAGE_SIZE)
        cpuData->eptPageTables[i] = physicalAddress | EPT_RWX | (EPT_POINTER_WB << 3);
    
    return eptp;
}

QWORD VmmCreateEptEntry(IN QWORD physicalAddress, IN QWORD access)
{
    return physicalAddress | access | (EPT_POINTER_WB << 3);
}