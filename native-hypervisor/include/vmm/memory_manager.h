#ifndef __HOST_MEMORY_MANAGER_H_
#define __HOST_MEMORY_MANAGER_H_

#include <types.h>
#include <error_codes.h>

#define VIRTUAL_ADDRESS_OFFET 0x640000000000ULL

VOID SetupVirtualAddress(IN QWORD pml4BaseAddress);
QWORD VirtualToPhysical(IN QWORD address);
QWORD PhysicalToVirtual(IN QWORD address);

#endif