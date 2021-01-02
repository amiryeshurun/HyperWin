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
    STATUS status;

    heap = &VmmGetVmmStruct()->currentCPU->sharedData->heap;
    if((status = heap->allocate(heap, sizeof(LIST_ENTRY), &newEntry)) != STATUS_SUCCESS)
    {
        Print("Could not allocate space for new list entry\n");
        return status;
    }
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
    return STATUS_SUCCESS;
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