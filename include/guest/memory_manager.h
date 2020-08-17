#ifndef __MEMORY_MANAGER_H_
#define __MEMORY_MANAGER_H_

#include <types.h>

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
        QWORD address : 40;
        QWORD reserved : 11;
        QWORD nx : 1;
    };
    QWORD entry;
}QWORD_PAGE_TABLE_ENTRY, *PQWORD_PAGE_TABLE_ENTRY;

QWORD CopyGuestMemory(OUT QWORD dest, IN QWORD src, IN DWORD length);
QWORD TranslateGuestVirtualToGuestPhysical(IN QWORD address);
QWORD TranslateGuestPhysicalToPhysicalAddress(IN QWORD address);

#endif