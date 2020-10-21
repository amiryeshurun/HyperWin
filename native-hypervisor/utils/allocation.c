#include <utils/allocation.h>

STATUS HeapAllocate(IN PHEAP heap, IN QWORD size, OUT BYTE_PTR* ptr)
{
    PHEAP_ENTRY heapEntry;
    for(heapEntry = heap->heap; ((heapEntry->status != HEAP_FREE) || (heapEntry->length < size)) && heapEntry->next;
         heapEntry = heapEntry->next->base);
    if(!heapEntry->next)
        return STATUS_COULD_NOT_ALLOCATE;
    heapEntry->length = size + sizeof(HEAP_ENTRY);
    heapEntry->next = heapEntry->base + heapEntry->length;
    heapEntry->status = HEAP_ALLOCATED;
    *ptr = heapEntry->base;
    heapEntry->next->base = heapEntry->next;
    heapEntry->next->length = heapEntry->length - size - sizeof(HEAP_ENTRY);
    heapEntry->next->status = HEAP_FREE;
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