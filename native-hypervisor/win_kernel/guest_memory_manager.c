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
    ASSERT(vmread(GUEST_CR4) & (1 << 5)); // PAE
    QWORD guestCR3 = vmread(GUEST_CR3);
    VIRTUAL_ADDRESS_PARTITIONING virtualAddress = (VIRTUAL_ADDRESS_PARTITIONING)(address & VIRTUAL_ADDRESS_MASK);
    PQWORD_PAGE_TABLE_ENTRY pml4Address = 
        (PQWORD_PAGE_TABLE_ENTRY)TranslateGuestPhysicalToHostVirtual((guestCR3 & VIRTUAL_ADDRESS_MASK)
             + virtualAddress.bitFields.pageMapLevel4Offset * sizeof(QWORD));
    if(!pml4Address->bitFields.valid)
        return STATUS_ADDRESS_NOT_VALID;
    PQWORD_PAGE_TABLE_ENTRY pdtAddress = 
        (PQWORD_PAGE_TABLE_ENTRY)TranslateGuestPhysicalToHostVirtual((pml4Address->bitFields.address << 12)
            + virtualAddress.bitFields.pageDirectoryPointerTableOffset * sizeof(QWORD));
    if(!pdtAddress->bitFields.valid)
        return STATUS_ADDRESS_NOT_VALID;
    PQWORD_PAGE_TABLE_ENTRY pdAddress = 
        (PQWORD_PAGE_TABLE_ENTRY)TranslateGuestPhysicalToHostVirtual((pdtAddress->bitFields.address << 12)
            + virtualAddress.bitFields.pageDirectoryOffset * sizeof(QWORD));
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
        if(!ptAddress->bitFields.valid)
            return STATUS_ADDRESS_NOT_VALID;
        QWORD guestPhysicalAddress = (ptAddress->bitFields.address << 12) + virtualAddress.bitFields.pageOffset;
    }
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
