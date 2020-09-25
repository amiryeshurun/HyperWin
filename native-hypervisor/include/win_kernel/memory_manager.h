#ifndef __GUEST_MEMORY_MANAGER_H_
#define __GUEST_MEMORY_MANAGER_H_

#include <types.h>

#define VIRTUAL_ADDRESS_MASK 0xffffffffffff
#define ADDRESS_NOT_VALID 0

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
    }bitFields;
    QWORD address;
}VIRTUAL_ADDRESS_PARTITIONING, PVIRTUAL_ADDRESS_PARTITIONING;

QWORD CopyGuestMemory(OUT QWORD_PTR dest, IN QWORD src, IN DWORD length);
QWORD TranslateGuestVirtualToGuestPhysical(IN QWORD address);
QWORD TranslateGuestPhysicalToPhysicalAddress(IN QWORD address);
QWORD TranslatePhysicalToHostVirtual(IN QWORD address);
QWORD TranslateGuestPhysicalToHostVirtual(IN QWORD address);

#endif