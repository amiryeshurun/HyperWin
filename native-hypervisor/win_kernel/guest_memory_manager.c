#include <win_kernel/memory_manager.h>
#include <vmm/memory_manager.h>
#include <vmm/vmcs.h>
#include <vmm/vm_operations.h>
#include <vmm/vmm.h>
#include <debug.h>

/* For more information about address translation, see Windows Internals, 7th edition, 
    page 381 */

STATUS WinMmTranslateGuestVirtualToGuestPhysical(IN QWORD address, OUT QWORD_PTR translatedAddress)
{
    return WinMmTranslateGuestVirtualToGuestPhysicalUsingCr3(address, translatedAddress, 0);
}

STATUS WinMmTranslateGuestVirtualToGuestPhysicalUsingCr3(IN QWORD address, OUT QWORD_PTR translatedAddress,
    IN QWORD guestCr3)
{
    QWORD_PAGE_TABLE_ENTRY pml4GuestCr3;
    QWORD guestPhysicalAddress;
    VIRTUAL_ADDRESS_PARTITIONING virtualAddress;
    PQWORD_PAGE_TABLE_ENTRY pml4Address, pdtAddress, pdAddress, ptAddress;

    ASSERT(vmread(GUEST_CR0) & CR0_PM_ENABLED); // Protected mode
    ASSERT(vmread(GUEST_CR0) & CR0_PG_ENABLED); // Paging
    ASSERT(vmread(GUEST_CR4) & CR4_PAE_ENABLED); // Physical address extension
#ifdef DEBUG_ADDRESS_TRANSLATION
    PrintDebugLevelDebug("Address: %8\n", address);
#endif
    pml4GuestCr3 = (QWORD_PAGE_TABLE_ENTRY)(guestCr3 ? guestCr3 : vmread(GUEST_CR3));
    virtualAddress = (VIRTUAL_ADDRESS_PARTITIONING)(address & VIRTUAL_ADDRESS_MASK);
    pml4Address = (PQWORD_PAGE_TABLE_ENTRY)WinMmTranslateGuestPhysicalToHostVirtual(pml4GuestCr3.bitFields.address << 12)
             + virtualAddress.bitFields.pageMapLevel4Offset;
#ifdef DEBUG_ADDRESS_TRANSLATION
    PrintDebugLevelDebug("PML4: %8 %8 %8\n", (pml4GuestCr3.bitFields.address << 12) + virtualAddress.bitFields.pageMapLevel4Offset 
        * sizeof(QWORD), pml4Address, *(QWORD_PTR)pml4Address);
#endif
    if(!pml4Address->bitFields.valid)
        return STATUS_ADDRESS_NOT_VALID;
    pdtAddress = (PQWORD_PAGE_TABLE_ENTRY)WinMmTranslateGuestPhysicalToHostVirtual(pml4Address->bitFields.address << 12)
            + virtualAddress.bitFields.pageDirectoryPointerTableOffset;
#ifdef DEBUG_ADDRESS_TRANSLATION
    PrintDebugLevelDebug("PDPT: %8 %8 %8\n", (pml4Address->bitFields.address << 12)
            + virtualAddress.bitFields.pageDirectoryPointerTableOffset * sizeof(QWORD), pdtAddress, *(QWORD_PTR)pdtAddress);
#endif
    if(!pdtAddress->bitFields.valid)
        return STATUS_ADDRESS_NOT_VALID;

    if(pdtAddress->bitFields.largePage) // 1-GB page
    {
        guestPhysicalAddress = (pdtAddress->bitFields.address << 12) + (address & VIRTUAL_ADDRESS_MASK
             & GB_PAGE_MASK);
        goto AddressTranslated;
    }
    pdAddress = (PQWORD_PAGE_TABLE_ENTRY)WinMmTranslateGuestPhysicalToHostVirtual(pdtAddress->bitFields.address << 12)
            + virtualAddress.bitFields.pageDirectoryOffset;
#ifdef DEBUG_ADDRESS_TRANSLATION
    PrintDebugLevelDebug("PDT: %8 %8 %8\n", (pdtAddress->bitFields.address << 12)
            + virtualAddress.bitFields.pageDirectoryOffset * sizeof(QWORD), pdAddress, *(QWORD_PTR)pdAddress);
#endif
    if(!pdAddress->bitFields.valid)
        return STATUS_ADDRESS_NOT_VALID;
    
    if(pdAddress->bitFields.largePage) // 2-MB page
    {
        guestPhysicalAddress = (pdAddress->bitFields.address << 12) + (address & VIRTUAL_ADDRESS_MASK
             & LARGE_PAGE_MASK);
        goto AddressTranslated;
    }
    // 4-KB page
    ptAddress = (PQWORD_PAGE_TABLE_ENTRY)WinMmTranslateGuestPhysicalToHostVirtual(pdAddress->bitFields.address << 12)
            + virtualAddress.bitFields.pageTableOffset;
#ifdef DEBUG_ADDRESS_TRANSLATION
    PrintDebugLevelDebug("PT: %8 %8 %8\n", (pdAddress->bitFields.address << 12)
                + virtualAddress.bitFields.pageTableOffset * sizeof(QWORD), ptAddress, *(QWORD_PTR)ptAddress);
#endif
    if(!ptAddress->bitFields.valid)
        return STATUS_ADDRESS_NOT_VALID;
    guestPhysicalAddress = (ptAddress->bitFields.address << 12) + virtualAddress.bitFields.pageOffset;

AddressTranslated:
#ifdef DEBUG_ADDRESS_TRANSLATION
    Print("Address was translated to guest physical: %8\n", guestPhysicalAddress);
#endif
    *translatedAddress = guestPhysicalAddress;
    return STATUS_SUCCESS;
}

QWORD WinMmTranslateGuestPhysicalToPhysicalAddress(IN QWORD address)
{
#ifdef DEBUG_ADDRESS_TRANSLATION || DEBUG_EPT_TRANSLATION
    Print("Translating guest physical address: %8\n", address);
#endif
    ASSERT(address / PAGE_SIZE <= COMPUTER_MEM_SIZE * ARRAY_PAGE_SIZE * ARRAY_PAGE_SIZE);
#ifdef DEBUG_ADDRESS_TRANSLATION || DEBUG_EPT_TRANSLATION
    Print("Translated to physical address: %8\n", 
        (GetVMMStruct()->currentCPU->eptPageTables[address / PAGE_SIZE] & REMOVE_PAGE_BITS) + (address % PAGE_SIZE));
#endif
    return (VmmGetVmmStruct()->currentCPU->eptPageTables[address / PAGE_SIZE] & REMOVE_PAGE_BITS) + (address % PAGE_SIZE);
}

QWORD WinMmTranslateGuestPhysicalToHostVirtual(IN QWORD address)
{
    return PhysicalToVirtual(WinMmTranslateGuestPhysicalToPhysicalAddress(address));
}

STATUS WinMmTranslateGuestVirtualToHostVirtual(IN QWORD address, OUT BYTE_PTR* hostAddress)
{
    QWORD guestPhysical;

    if(WinMmTranslateGuestVirtualToGuestPhysical(address, &guestPhysical) != STATUS_SUCCESS)
        return STATUS_ADDRESS_NOT_VALID;
    *hostAddress = WinMmTranslateGuestPhysicalToHostVirtual(guestPhysical);
    return STATUS_SUCCESS;
}

STATUS WinMmCopyGuestMemory(OUT BYTE_PTR dest, IN QWORD src, IN QWORD length)
{
    QWORD hostVirtual, alignedLengthUntilNextPage;

    for(QWORD offset = 0, increament = 0; TRUE; offset += increament)
    {
        if(WinMmTranslateGuestVirtualToHostVirtual(src + offset, &hostVirtual) != STATUS_SUCCESS)
            return STATUS_ADDRESS_NOT_VALID;
        alignedLengthUntilNextPage = (src + offset) % PAGE_SIZE ? ALIGN_UP(src + offset, PAGE_SIZE) - src : PAGE_SIZE;
        if(length <= alignedLengthUntilNextPage)
        {
            HwCopyMemory(dest, hostVirtual, length);
            break;
        }
        // Copy the current page (part/whole)
        HwCopyMemory(dest, hostVirtual, alignedLengthUntilNextPage);
        increament = alignedLengthUntilNextPage;
        dest += alignedLengthUntilNextPage;
        length -= alignedLengthUntilNextPage;
    }
    return STATUS_SUCCESS;
}

STATUS WinMmCopyMemoryToGuest(IN QWORD dest, IN BYTE_PTR src, IN QWORD length)
{
    QWORD hostVirtual, alignedLengthUntilNextPage;

    for(QWORD offset = 0, increament = 0; TRUE; offset += increament)
    {
        if(WinMmTranslateGuestVirtualToHostVirtual(dest + offset, &hostVirtual) != STATUS_SUCCESS)
            return STATUS_ADDRESS_NOT_VALID;
        alignedLengthUntilNextPage = (dest + offset) % PAGE_SIZE ? ALIGN_UP(dest + offset, PAGE_SIZE) - dest : PAGE_SIZE;
        if(length <= alignedLengthUntilNextPage)
        {
            HwCopyMemory(hostVirtual, src, length);
            break;
        }
        // Copy the current page (part/whole)
        HwCopyMemory(hostVirtual, src, alignedLengthUntilNextPage);
        increament = alignedLengthUntilNextPage;
        src += alignedLengthUntilNextPage;
        length -= alignedLengthUntilNextPage;
    }
    return STATUS_SUCCESS;
}
