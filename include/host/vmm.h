#ifndef __VMM_H_
#define __VMM_H_

#include <types.h>

#define PAGE_SIZE 0x1000
#define LARGE_PAGE_SIZE 0x200000
#define ARRAY_PAGE_SIZE (PAGE_SIZE / sizeof(QWORD))
#define COMPUTER_MEM_SIZE 16

#define STACK_SIZE (4 * PAGE_SIZE)
#define HEAP_SIZE (4 * PAGE_SIZE)

#define MAX_CORES 16

struct _SINGLE_CPU_DATA;

typedef struct _SHARED_CPU_DATA
{
    struct _SINGLE_CPU_DATA* cpuData[MAX_CORES];
} SHARED_CPU_DATA, *PSHARED_CPU_DATA;

typedef struct _SINGLE_CPU_DATA
{
    BYTE vmcs[PAGE_SIZE];
    BYTE stack[STACK_SIZE];
    BYTE heap[HEAP_SIZE];
    QWORD pageMapLevel4s[ARRAY_PAGE_SIZE]; // cr3
    QWORD pageDirectoryPointerTables[ARRAY_PAGE_SIZE]; // 1 PML entry = 512GB, enough for the HV
    QWORD pageDirectories[ARRAY_PAGE_SIZE * ARRAY_PAGE_SIZE]; // HV will use 2MB pages
    QWORD eptPageMapLevel4s[ARRAY_PAGE_SIZE];
    QWORD eptPageDirectoryPointerTables[ARRAY_PAGE_SIZE];
    /* Each 512 entries here are referenced by an entry from the previous table. 
        Each entry in the previous table maps 1GB of memory */
    QWORD eptPageDirectories[ARRAY_PAGE_SIZE * COMPUTER_MEM_SIZE]; 
    QWORD eptPageTables[ARRAY_PAGE_SIZE * ARRAY_PAGE_SIZE * COMPUTER_MEM_SIZE];
    PSHARED_CPU_DATA sharedData;
} SINGLE_CPU_DATA, *PSINGLE_CPU_DATA;

typedef struct _CURRENT_GUEST_STATE
{
    REGISTERS guestRegisters;
    PSINGLE_CPU_DATA currentCPU;
} CURRENT_GUEST_STATE, *PCURRENT_GUEST_STATE;

#endif