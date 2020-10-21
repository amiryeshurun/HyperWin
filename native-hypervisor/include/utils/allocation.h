#ifndef __ALLOCATION_H_
#define __ALLOCATION_H_

#include <x86_64.h>
#include <types.h>
#include <error_codes.h>

#define HEAP_SIZE (8 * PAGE_SIZE)

typedef struct _HEAP;

typedef STATUS (*ALLOCATION_FUNCTION)(IN struct _HEAP*, IN QWORD, OUT BYTE_PTR*);
typedef STATUS (*DEALLOCATION_FUNCTION)(IN struct _HEAP*, IN BYTE_PTR);
typedef STATUS (*DEFRAGMENT_HEAP)(IN struct _HEAP*);

/* Heap entry statuses */
#define HEAP_FREE 0
#define HEAP_ALLOCATED 1

typedef struct _HEAP_ENTRY
{
    BYTE_PTR base;
    QWORD status;
    QWORD length;
    struct _HEAP_ENTRY* next;
} HEAP_ENTRY, *PHEAP_ENTRY;

typedef struct _HEAP
{
    BYTE heap[HEAP_SIZE];
    QWORD freesCount;
    QWORD freesCycle;
    ALLOCATION_FUNCTION allocate;
    DEALLOCATION_FUNCTION deallocate;
    DEFRAGMENT_HEAP defragment;
} HEAP, *PHEAP;

#endif