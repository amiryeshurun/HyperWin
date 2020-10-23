#include <utils/allocation.h>
#include <debug.h>

VOID HeapInit(IN PHEAP heap, IN QWORD length, IN QWORD freesCycle, IN ALLOCATION_FUNCTION allocFunc,
    IN DEALLOCATION_FUNCTION deallocFunc, IN DEFRAGMENT_HEAP defragFunc)
{
    heap->freesCycle = freesCycle;
    heap->allocate = allocFunc;
    heap->deallocate = deallocFunc;
    heap->defragment = defragFunc;
    PHEAP_ENTRY heapStart = heap->heap;
    heapStart->base = heap->heap;
    heapStart->length = length;
    heapStart->status = HEAP_FREE;
    heapStart->next = NULL;
}

STATUS HeapAllocate(IN PHEAP heap, IN QWORD size, OUT BYTE_PTR* ptr)
{
    PHEAP_ENTRY heapEntry;
    for(heapEntry = heap->heap; (heapEntry->status != HEAP_FREE || (size + sizeof(HEAP_ENTRY)) > heapEntry->length) && heapEntry->next;
         heapEntry = heapEntry->next->base);
    if(heapEntry->status != HEAP_FREE)
        return STATUS_HEAP_FULL;
    QWORD currentEntryLength = heapEntry->length;
    BYTE_PTR currentNext = heapEntry->next;
    if(currentEntryLength == size + sizeof(HEAP_ENTRY))
    {
        heapEntry->status = HEAP_ALLOCATED;
        *ptr = heapEntry->base + sizeof(HEAP_ENTRY);
        return STATUS_SUCCESS;
    }
    heapEntry->length = size + sizeof(HEAP_ENTRY);
    heapEntry->next = heapEntry->base + heapEntry->length;
    heapEntry->status = HEAP_ALLOCATED;
    *ptr = heapEntry->base + sizeof(HEAP_ENTRY);
    heapEntry->next->base = heapEntry->next;
    heapEntry->next->length = currentEntryLength - heapEntry->length;
    heapEntry->next->status = HEAP_FREE;
    heapEntry->next->next = currentNext;
    return STATUS_SUCCESS;
}

STATUS HeapDeallocate(IN PHEAP heap, IN BYTE_PTR ptr)
{
    PHEAP_ENTRY heapEntry = ptr - sizeof(HEAP_ENTRY);
    if(heapEntry->status != HEAP_ALLOCATED)
        return STATUS_UNALLOCATED_MEMORY;
    heapEntry->status = HEAP_FREE;
    if(++(heap->freesCount) >= heap->freesCycle)
    {
        if(heap->defragment(heap) != STATUS_SUCCESS)
            return STATUS_DEFRAGMENTATION_FAILED;
        heap->freesCount = 0;
    }
    return STATUS_SUCCESS;
}

STATUS HeapDefragment(IN PHEAP heap)
{
    PHEAP_ENTRY currentEntry = heap->heap;
    while(currentEntry)
    {
        PHEAP_ENTRY start = currentEntry, end = start;
        QWORD totalLength = 0;
        for(;end->next && end->status == HEAP_FREE && end->next->status == HEAP_FREE; totalLength += end->length, 
            end = end->next);
        if(end->status == HEAP_FREE)
            totalLength += end->length;
        if(start == end) // only one free entry OR the entry was not free
        {
            currentEntry = end->next;
            continue;
        }
        start->length = totalLength;
        start->next = end->next;
        currentEntry = end->next;
    }
}

VOID HeapDump(IN PHEAP heap)
{
    PHEAP_ENTRY curr = heap->heap;
    Print("Dump start:\n");
    while(curr)
    {
        Print("Entry Start #############\nStart: %8\nLength: %8\nStatus: %8\nNext: %8\n#############\n", curr->base, curr->length,
            curr->status, curr->next);
        curr = curr->next;
    }
}