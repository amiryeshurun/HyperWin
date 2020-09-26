#ifndef __HOST_MEMORY_MANAGER_H_
#define __HOST_MEMORY_MANAGER_H_

#include <types.h>
#include <error_codes.h>
#include <vmm/vmm.h>

#define VIRTUAL_ADDRESS_OFFET 0x640000000000ULL

#define PAGE_PRESENT 1
#define PAGE_RW (1 << 1)
#define PAGE_LARGE_PAGE (1 << 7)

VOID SetupVirtualAddress(IN QWORD pml4BaseAddress);
QWORD VirtualToPhysical(IN QWORD address);
QWORD PhysicalToVirtual(IN QWORD address);
QWORD InitializeHypervisorPaging(IN PSINGLE_CPU_DATA cpuData);

#endif