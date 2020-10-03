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
    QWORD guestCR3 = vmread(GUEST_CR3);
    Print("cr3 %8\n", guestCR3);
    VIRTUAL_ADDRESS_PARTITIONING virtualAddress = (VIRTUAL_ADDRESS_PARTITIONING)(address & VIRTUAL_ADDRESS_MASK);
    Print("virtAddr: %8\n", virtualAddress);
    PQWORD_PAGE_TABLE_ENTRY pml4Address = 
        (PQWORD_PAGE_TABLE_ENTRY)TranslateGuestPhysicalToHostVirtual(guestCR3 & VIRTUAL_ADDRESS_MASK)
             + virtualAddress.bitFields.pageMapLevel4Offset;
    Print("pml4: %8\n", pml4Address);
    if(!pml4Address->bitFields.valid)
        return STATUS_ADDRESS_NOT_VALID;
    PQWORD_PAGE_TABLE_ENTRY pdtAddress = 
        (PQWORD_PAGE_TABLE_ENTRY)TranslateGuestPhysicalToHostVirtual(pml4Address->bitFields.address << 12)
            + virtualAddress.bitFields.pageDirectoryPointerTableOffset;
    Print("pdt: %8\n", pdtAddress);
    if(!pdtAddress->bitFields.valid)
        return STATUS_ADDRESS_NOT_VALID;
    PQWORD_PAGE_TABLE_ENTRY pdAddress = 
        (PQWORD_PAGE_TABLE_ENTRY)TranslateGuestPhysicalToHostVirtual(pdtAddress->bitFields.address << 12)
            + virtualAddress.bitFields.pageDirectoryOffset;
    Print("pdAddress: %8\n", pdAddress);
    if(!pdAddress->bitFields.valid)
        return STATUS_ADDRESS_NOT_VALID;
    PQWORD_PAGE_TABLE_ENTRY ptAddress = 
        (PQWORD_PAGE_TABLE_ENTRY)TranslateGuestPhysicalToHostVirtual(pdAddress->bitFields.address << 12)
            + virtualAddress.bitFields.pageTableOffset;
    Print("ptAddress: %8\n", ptAddress);
    if(!ptAddress->bitFields.valid)
        return STATUS_ADDRESS_NOT_VALID;
    QWORD guestPhysicalAddress = (ptAddress->bitFields.address << 12) + virtualAddress.bitFields.pageOffset;
    Print("gphys: %8\n", guestPhysicalAddress);
    *translatedAddress = guestPhysicalAddress;
    return STATUS_SUCCESS;
}

QWORD TranslateGuestPhysicalToPhysicalAddress(IN QWORD address)
{
    ASSERT(address / PAGE_SIZE <= COMPUTER_MEM_SIZE * ARRAY_PAGE_SIZE * ARRAY_PAGE_SIZE);
    return GetVMMStruct()->currentCPU->eptPageTables[address / PAGE_SIZE] & REMOVE_PAGE_BITS  + (address % PAGE_SIZE);
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
        Print("len: %d, alig: %d\n", length, alignedLengthUntilNextPage);
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
