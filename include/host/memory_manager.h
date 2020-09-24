#ifndef __HOST_MEMORY_MANAGER_H_
#define __HOST_MEMORY_MANAGER_H_

#include <types.h>
#include <error_codes.h>

#define E820_USABLE_REGION 1
#define E820_RESERVED_REGION 2
#define E820_RECLAIMABLE_REGION 3
#define E820_NVS_REGION 4
#define E820_BAD_MEMORY_REGION 5

#define E820_OUTPUT_MAX_ENTRIES 30

typedef struct _E820_LIST_ENTRY
{
    QWORD baseAddress;
    QWORD length;
    DWORD type;
    DWORD extendedAttribute;
}E820_LIST_ENTRY, *PE820_LIST_ENTRY;

STATUS AllocateMemoryUsingMemoryMap
    (IN PE820_LIST_ENTRY memoryMap, IN DWORD memoryRegionsCount, IN QWORD allocationSize, OUT BYTE_PTR* address);
QWORD VirtualToPhysical(IN QWORD address);
QWORD PhysicalToVirtual(IN QWORD address);

#endif