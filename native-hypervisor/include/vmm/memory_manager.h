#ifndef __HOST_MEMORY_MANAGER_H_
#define __HOST_MEMORY_MANAGER_H_

#include <types.h>
#include <error_codes.h>
#include <vmm/vmm.h>

#define VIRTUAL_ADDRESS_OFFET 0x640000000000ULL
#define VIRTUAL_ADDRESS_MASK 0xffffffffffffULL
#define REMOVE_PAGE_BITS 0xfffffffff000ULL

#define PAGE_PRESENT 1
#define PAGE_RW (1 << 1)
#define PAGE_LARGE_PAGE (1 << 7)


/* EPT Configurations */
#define EPT_POINTER_WB 6
#define EPT_PAGE_WALK_LENGTH (3 << 3)
#define EPT_READ 1
#define EPT_WRITE (1 << 1)
#define EPT_EXECUTE (1 << 2)
#define EPT_RWX (EPT_READ | EPT_WRITE | EPT_EXECUTE)
#define EPT_RW (EPT_READ | EPT_WRITE)
#define EPT_WX (EPT_WRITE | EPT_EXECUTE)
#define EPT_RX (EPT_READ | EPT_EXECUTE)
#define EPT_ACCESS_MASK (~(7ULL))

typedef union _EXTENDED_PAGE_TABLE_POINTER {
    QWORD value;
    struct 
    {
        QWORD memoryType : 3; 
        QWORD pageWalkLength : 3;
        QWORD dirtyAndAceessEnabled : 1; 
        QWORD reserved : 5; 
        QWORD pml4Address : 36;
        QWORD reserved1 : 16;
    }bitFields;
}__attribute__((__packed__)) EXTENDED_PAGE_TABLE_POINTER, *PEXTENDED_PAGE_TABLE_POINTER;


typedef union _EPT_PML4E_PDPTE_PDE
{
	QWORD value;
	struct
    {
		QWORD read : 1;
		QWORD write : 1;
		QWORD execute : 1;
		QWORD reserved : 5; // MBZ
		QWORD accessed : 1;
		QWORD ignored : 1;
		QWORD executeForUserMode : 1;
		QWORD ignored1 : 1;
		QWORD physicalAddress : 36;
		QWORD reserved1 : 4;
		QWORD ignored2 : 12;
	}bitFields;
}__attribute__((__packed__)) EPT_PML4E_PDPTE_PDE, *PEPT_PML4E_PDPTE_PDE;

typedef union _EPT_PTE 
{
	QWORD value;
	struct {
		QWORD read : 1;
		QWORD write : 1;
		QWORD execute : 1;
		QWORD eptMemoryType : 3;
		QWORD ignorePAT : 1;
		QWORD ignored : 1;
		QWORD accessedFlag : 1;	
		QWORD dirtyFlag : 1;
		QWORD executeForUserMode : 1;
		QWORD ignored1 : 1;
		QWORD physicalAddress : 36;
		QWORD reserved : 4;
		QWORD ignored2 : 11;
		QWORD suppressVE : 1;
	}bitFields;
}__attribute__((__packed__)) EPT_PTE, *PEPT_PTE;

VOID VmmSetupVirtualAddress(IN QWORD pml4BaseAddress);
QWORD VirtualToPhysical(IN BYTE_PTR address);
BYTE_PTR PhysicalToVirtual(IN QWORD address);
QWORD VmmInitializeHypervisorPaging(IN PSINGLE_CPU_DATA cpuData);
QWORD VmmInitializeExtendedPageTable(IN PSINGLE_CPU_DATA cpuData);
QWORD VmmCreateEptEntry(IN QWORD physicalAddress, IN QWORD access);

#endif