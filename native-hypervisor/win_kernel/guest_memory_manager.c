#include <win_kernel/memory_manager.h>
#include <vmm/memory_manager.h>
#include <vmm/vmcs.h>
#include <vmm/vm_operations.h>
#include <vmm/vmm.h>
#include <debug.h>

/* For more information about address translation, see Windows Internals, 7th edition, 
    page 381 */
STATUS TranslateGuestVirtualToGuestPhysical(IN QWORD address, OUT QWORD_PTR translatedAddress)
{
    ASSERT(vmread(GUEST_CR0) & CR0_PM_ENABLED); // Protected mode
    ASSERT(vmread(GUEST_CR0) & CR0_PG_ENABLED); // Paging
    ASSERT(vmread(GUEST_CR4) & CR4_PAE_ENABLED); // Physical address extension
#ifdef DEBUG_ADDRESS_TRANSLATION
    PrintDebugLevelDebug("Address: %8\n", address);
#endif
    QWORD guestCR3 = vmread(GUEST_CR3);
    VIRTUAL_ADDRESS_PARTITIONING virtualAddress = (VIRTUAL_ADDRESS_PARTITIONING)(address & VIRTUAL_ADDRESS_MASK);
    PQWORD_PAGE_TABLE_ENTRY pml4Address = 
        (PQWORD_PAGE_TABLE_ENTRY)TranslateGuestPhysicalToHostVirtual((guestCR3 & VIRTUAL_ADDRESS_MASK)
             + virtualAddress.bitFields.pageMapLevel4Offset * sizeof(QWORD));
#ifdef DEBUG_ADDRESS_TRANSLATION
    PrintDebugLevelDebug("PML4: %8 %8 %8\n", (guestCR3 & VIRTUAL_ADDRESS_MASK)
             + virtualAddress.bitFields.pageMapLevel4Offset * sizeof(QWORD), pml4Address, *(QWORD_PTR)pml4Address);
#endif
    if(!pml4Address->bitFields.valid)
        return STATUS_ADDRESS_NOT_VALID;
    PQWORD_PAGE_TABLE_ENTRY pdtAddress = 
        (PQWORD_PAGE_TABLE_ENTRY)TranslateGuestPhysicalToHostVirtual((pml4Address->bitFields.address << 12)
            + virtualAddress.bitFields.pageDirectoryPointerTableOffset * sizeof(QWORD));
#ifdef DEBUG_ADDRESS_TRANSLATION
    PrintDebugLevelDebug("PDPT: %8 %8 %8\n", (pml4Address->bitFields.address << 12)
            + virtualAddress.bitFields.pageDirectoryPointerTableOffset * sizeof(QWORD), pdtAddress, *(QWORD_PTR)pdtAddress);
#endif
    if(!pdtAddress->bitFields.valid)
        return STATUS_ADDRESS_NOT_VALID;
    PQWORD_PAGE_TABLE_ENTRY pdAddress = 
        (PQWORD_PAGE_TABLE_ENTRY)TranslateGuestPhysicalToHostVirtual((pdtAddress->bitFields.address << 12)
            + virtualAddress.bitFields.pageDirectoryOffset * sizeof(QWORD));
#ifdef DEBUG_ADDRESS_TRANSLATION
    PrintDebugLevelDebug("PDT: %8 %8 %8\n", (pdtAddress->bitFields.address << 12)
            + virtualAddress.bitFields.pageDirectoryOffset * sizeof(QWORD), pdAddress, *(QWORD_PTR)pdAddress);
#endif
    if(!pdAddress->bitFields.valid)
        return STATUS_ADDRESS_NOT_VALID;
    QWORD guestPhysicalAddress;
    if(pdAddress->bitFields.largePage) // 2-MB page
        guestPhysicalAddress = (pdAddress->bitFields.address << 12) + (address & VIRTUAL_ADDRESS_MASK
             & LARGE_PAGE_MASK); 
    else // 4-KB page
    {
        PQWORD_PAGE_TABLE_ENTRY ptAddress = 
            (PQWORD_PAGE_TABLE_ENTRY)TranslateGuestPhysicalToHostVirtual((pdAddress->bitFields.address << 12)
                + virtualAddress.bitFields.pageTableOffset * sizeof(QWORD));
#ifdef DEBUG_ADDRESS_TRANSLATION
    PrintDebugLevelDebug("PT: %8 %8 %8\n", (pdAddress->bitFields.address << 12)
                + virtualAddress.bitFields.pageTableOffset * sizeof(QWORD), ptAddress, *(QWORD_PTR)ptAddress);
#endif
        if(!ptAddress->bitFields.valid)
            return STATUS_ADDRESS_NOT_VALID;
        guestPhysicalAddress = (ptAddress->bitFields.address << 12) + virtualAddress.bitFields.pageOffset;
    }
#ifdef DEBUG_ADDRESS_TRANSLATION
    Print("Address was translated to guest physical: %8\n", guestPhysicalAddress);
#endif
    *translatedAddress = guestPhysicalAddress;
    return STATUS_SUCCESS;
}

QWORD TranslateGuestPhysicalToPhysicalAddress(IN QWORD address)
{
    ASSERT(address / PAGE_SIZE <= COMPUTER_MEM_SIZE * ARRAY_PAGE_SIZE * ARRAY_PAGE_SIZE);
    return (GetVMMStruct()->currentCPU->eptPageTables[address / PAGE_SIZE] & REMOVE_PAGE_BITS) + (address % PAGE_SIZE);
}

QWORD TranslateGuestPhysicalToHostVirtual(IN QWORD address)
{
    return PhysicalToVirtual(TranslateGuestPhysicalToPhysicalAddress(address));
}

STATUS TranslateGuestVirtualToHostVirtual(IN QWORD address, OUT QWORD_PTR hostAddress)
{
    QWORD guestPhysical;
    if(TranslateGuestVirtualToGuestPhysical(address, &guestPhysical) != STATUS_SUCCESS)
        return STATUS_ADDRESS_NOT_VALID;
    *hostAddress = TranslateGuestPhysicalToHostVirtual(guestPhysical);
    return STATUS_SUCCESS;
}

STATUS CopyGuestMemory(OUT BYTE_PTR dest, IN QWORD src, IN QWORD length)
{
    QWORD hostVirtual;
    for(QWORD offset = 0, increament = 0; TRUE; offset += increament)
    {
        if(TranslateGuestVirtualToHostVirtual(src + offset, &hostVirtual) != STATUS_SUCCESS)
            return STATUS_ADDRESS_NOT_VALID;
        QWORD alignedLengthUntilNextPage = (src + offset) % PAGE_SIZE ? 
            ALIGN_UP(src + offset, PAGE_SIZE) - src : PAGE_SIZE;
        if(length <= alignedLengthUntilNextPage)
        {
            CopyMemory(dest, hostVirtual, length);
            break;
        }
        // Copy the current page (part/whole)
        CopyMemory(dest, hostVirtual, alignedLengthUntilNextPage);
        increament = alignedLengthUntilNextPage;
        dest += alignedLengthUntilNextPage;
        length -= alignedLengthUntilNextPage;
    }
    return STATUS_SUCCESS;
}

STATUS CopyMemoryToGuest(IN QWORD dest, IN BYTE_PTR src, IN QWORD length)
{
    QWORD hostVirtual;
    for(QWORD offset = 0, increament = 0; TRUE; offset += increament)
    {
        if(TranslateGuestVirtualToHostVirtual(dest + offset, &hostVirtual) != STATUS_SUCCESS)
            return STATUS_ADDRESS_NOT_VALID;
        QWORD alignedLengthUntilNextPage = (dest + offset) % PAGE_SIZE ? 
            ALIGN_UP(dest + offset, PAGE_SIZE) - dest : PAGE_SIZE;
        if(length <= alignedLengthUntilNextPage)
        {
            CopyMemory(hostVirtual, src, length);
            break;
        }
        // Copy the current page (part/whole)
        CopyMemory(hostVirtual, src, alignedLengthUntilNextPage);
        increament = alignedLengthUntilNextPage;
        src += alignedLengthUntilNextPage;
        length -= alignedLengthUntilNextPage;
    }
    return STATUS_SUCCESS;
}
