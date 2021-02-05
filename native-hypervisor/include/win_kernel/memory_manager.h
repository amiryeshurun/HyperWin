#ifndef __GUEST_MEMORY_MANAGER_H_
#define __GUEST_MEMORY_MANAGER_H_

#include <types.h>
#include <error_codes.h>

#define VIRTUAL_ADDRESS_MASK 0xffffffffffffULL
#define REMOVE_PAGE_BITS 0xfffffffff000ULL

typedef union _QWORD_PAGE_TABLE_ENTRY 
{
    struct 
    {
        QWORD valid : 1;
        QWORD write : 1;
        QWORD owner : 1;
        QWORD writeThrough : 1;
        QWORD cacheDisabled : 1;
        QWORD accessed : 1;
        QWORD dirty : 1;
        QWORD largePage : 1;
        QWORD global : 1;
        QWORD copyOnWrite : 1;
        QWORD prototype: 1;
        QWORD writeSoftware : 1;
        QWORD address : 36;
        QWORD reserved : 15;
        QWORD nx : 1;
    }bitFields;
    QWORD entry;
}QWORD_PAGE_TABLE_ENTRY, *PQWORD_PAGE_TABLE_ENTRY;

typedef union _VIRTUAL_ADDRESS_PARTITIONING
{
    struct{
        QWORD pageOffset : 12;
        QWORD pageTableOffset : 9;
        QWORD pageDirectoryOffset : 9;
        QWORD pageDirectoryPointerTableOffset : 9;
        QWORD pageMapLevel4Offset : 9;
        QWORD signExtension : 16;
    }bitFields;
    QWORD address;
}VIRTUAL_ADDRESS_PARTITIONING, PVIRTUAL_ADDRESS_PARTITIONING;

STATUS WinMmCopyGuestMemory(OUT BYTE_PTR dest, IN QWORD src, IN QWORD length);
STATUS WinMmTranslateGuestVirtualToGuestPhysicalUsingCr3(IN QWORD address, OUT QWORD_PTR translatedAddress,
    IN QWORD guestCr3);
STATUS WinMmTranslateGuestVirtualToGuestPhysical(IN QWORD address, OUT QWORD_PTR translatedAddress);
QWORD WinMmTranslateGuestPhysicalToPhysicalAddress(IN QWORD address);
QWORD WinMmTranslateGuestPhysicalToHostVirtual(IN QWORD address);
STATUS WinMmCopyMemoryToGuest(IN QWORD dest, IN BYTE_PTR src, IN QWORD length);
STATUS WinMmTranslateGuestVirtualToHostVirtual(IN QWORD address, OUT BYTE_PTR* hostAddress);

#endif