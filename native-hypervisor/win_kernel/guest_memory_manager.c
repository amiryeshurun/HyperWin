#include <win_kernel/memory_manager.h>
#include <vmm/vmcs.h>
#include <vmm/vm_operations.h>

/* For more information about address translation, see Windows Internals, 7th edition, 
    page 381 */
QWORD TranslateGuestVirtualToGuestPhysical(IN QWORD address)
{
    QWORD guestCR3 = vmread(GUEST_CR3);
    VIRTUAL_ADDRESS_PARTITIONING virtualAddress = (VIRTUAL_ADDRESS_PARTITIONING)(address & VIRTUAL_ADDRESS_MASK);
    PQWORD_PAGE_TABLE_ENTRY pml4Address = 
        (PQWORD_PAGE_TABLE_ENTRY)TranslateGuestPhysicalToHostVirtual(guestCR3)
             + virtualAddress.bitFields.pageMapLevel4Offset;
    if(!pml4Address->bitFields.valid)
        return ADDRESS_NOT_VALID;
    PQWORD_PAGE_TABLE_ENTRY pdtAddress = 
        (PQWORD_PAGE_TABLE_ENTRY)TranslateGuestPhysicalToHostVirtual(pml4Address->bitFields.address)
            + virtualAddress.bitFields.pageDirectoryPointerTableOffset;
    if(!pdtAddress->bitFields.valid)
        return ADDRESS_NOT_VALID;
    PQWORD_PAGE_TABLE_ENTRY pdAddress = 
        (PQWORD_PAGE_TABLE_ENTRY)TranslateGuestPhysicalToHostVirtual(pdtAddress->bitFields.address)
            + virtualAddress.bitFields.pageDirectoryOffset;
    if(!pdAddress->bitFields.valid)
        return ADDRESS_NOT_VALID;
    PQWORD_PAGE_TABLE_ENTRY ptAddress = 
        (PQWORD_PAGE_TABLE_ENTRY)TranslateGuestPhysicalToHostVirtual(pdAddress->bitFields.address)
            + virtualAddress.bitFields.pageTableOffset;
    if(!ptAddress->bitFields.valid)
        return ADDRESS_NOT_VALID;
    QWORD guestPhysicalAddress = TranslateGuestPhysicalToHostVirtual(ptAddress->bitFields.address)
        + virtualAddress.bitFields.pageOffset;
    return guestPhysicalAddress;
}

QWORD CopyGuestMemory(OUT QWORD_PTR dest, IN QWORD src, IN DWORD length)
{
    return 0;
}

QWORD TranslateGuestPhysicalToPhysicalAddress(IN QWORD address)
{
    return 0;
}

QWORD TranslatePhysicalToHostVirtual(IN QWORD address)
{
    return 0;
}

QWORD TranslateGuestPhysicalToHostVirtual(IN QWORD address)
{
    return 0;
}