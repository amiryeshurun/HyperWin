#include <utils/list.h>
#include <debug.h>
#include <vmm/vmm.h>
#include <utils/allocation.h>

STATUS ListCreate(OUT PLIST list)
{
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;

    return STATUS_SUCCESS;
}

STATUS ListInsert(IN PLIST list, IN QWORD data)
{
    PLIST_ENTRY last, newEntry;
    PHEAP heap;
    STATUS status = STATUS_SUCCESS;

    heap = &VmmGetVmmStruct()->currentCPU->sharedData->heap;
    SUCCESS_OR_CLEANUP(heap->allocate(heap, sizeof(LIST_ENTRY), &newEntry));
    // Is the list empty?
    if(list->head == NULL)
    {
        list->head = newEntry;
        list->tail = newEntry;
        newEntry->prev = NULL;
        newEntry->next = NULL;
    }
    else
    {
        last = list->tail;
        last->next = newEntry;
        newEntry->prev = last;
        newEntry->next = NULL;
        list->tail = newEntry;
    }
    newEntry->data = data;
    list->size++;

cleanup:
    return status;
}

BOOL ListContains(IN PLIST lise, IN QWORD data)
{
    PLIST_ENTRY listEntry;

    do
    {
        if(listEntry->data == data)
            return TRUE;
    } while(listEntry->next);

    return FALSE;
}

STATUS ListRemove(IN PLIST list, IN QWORD data)
{
    PLIST_ENTRY begin, next;
    PHEAP heap;

    heap = &VmmGetVmmStruct()->currentCPU->sharedData->heap;
    begin = list->head;
    while(begin)
    {
        if(begin->data == data)
        {
            if(begin->prev)
                begin->prev->next = begin->next;
            if(begin->next)
                begin->next->prev = begin->prev;
            next = begin->next;
            heap->deallocate(heap, begin);
        }
        else
            next = begin->next;
        begin = next;
    }
    return STATUS_SUCCESS;
}