#include <utils/array.h>
#include <vmm/vmm.h>
#include <debug.h>

STATUS QPArrayInit(OUT PQWORD_PAIRS_ARRAY array)
{
    PHEAP heap;

    heap = &VmmGetVmmStruct()->currentCPU->sharedData->heap;
    if(heap->allocate(heap, sizeof(PQWORD_PAIR), &array->arr))
    {
        Print("Could not initialize array\n");
        return STATUS_NO_MEM_AVAILABLE;
    }
    array->size = 1;
    array->count = 0;
    return STATUS_SUCCESS;
}

STATUS QPArrayInsert(IN PQWORD_PAIRS_ARRAY array, IN PQWORD_PAIR value)
{
    PHEAP heap;
    PQWORD_PAIR* newArr;

    heap = &VmmGetVmmStruct()->currentCPU->sharedData->heap;
    if(array->size == array->count)
    {
        if(heap->allocate(heap, array->size * 2 * sizeof(PQWORD_PAIR), &newArr) != STATUS_SUCCESS)
        {
            Print("Could not insert to array\n");
            return STATUS_NO_MEM_AVAILABLE;
        }
        for(QWORD i = 0; i < array->count; i++)
            newArr[i] = array->arr[i];
        newArr[array->count++] = value;
        array->size *= 2;
        heap->deallocate(heap, array->arr);
        array->arr = newArr;
    }
    else
        array->arr[array->count++] = value;
    return STATUS_SUCCESS;
}

QWORD QPArrayRemove(IN PQWORD_PAIRS_ARRAY array, IN QWORD key)
{
    PHEAP heap;
    QWORD ret = VALUE_NOT_FOUND;

    heap = &VmmGetVmmStruct()->currentCPU->sharedData->heap;
    for(QWORD i = 0; i < array->count; i++)
    {
        if(array->arr[i]->key == key)
        {
            ret = array->arr[i]->value;
            heap->deallocate(heap, array->arr[i]);
            for(QWORD j = i; j < array->count - 1; j++)
                array->arr[j] = array->arr[j + 1];
            array->count--;
            i--;
            return ret;
        }
    }

    return ret;
}

STATUS QArrayInit(OUT PQWORD_ARRAY array)
{
    PHEAP heap;
   
    heap = &VmmGetVmmStruct()->currentCPU->sharedData->heap;
    if(heap->allocate(heap, sizeof(QWORD), &array->arr) != STATUS_SUCCESS)
    {
        Print("Could not initialize array\n");
        return STATUS_NO_MEM_AVAILABLE;
    }
    array->size = 1;
    array->count = 0;
    
    return STATUS_SUCCESS;
}

STATUS QArrayInsert(IN PQWORD_ARRAY array, IN QWORD value)
{
    PHEAP heap;
    QWORD_PTR newArr;

    heap = &VmmGetVmmStruct()->currentCPU->sharedData->heap;
    if(array->size == array->count)
    {
        if(heap->allocate(heap, array->size * 2 * sizeof(QWORD), &newArr) != STATUS_SUCCESS)
        {
            Print("Could not insert to array\n");
            return STATUS_NO_MEM_AVAILABLE;
        }
        for(QWORD i = 0; i < array->count; i++)
            newArr[i] = array->arr[i];
        newArr[array->count++] = value;
        array->size *= 2;
        heap->deallocate(heap, array->arr);
        array->arr = newArr;
    }
    else
        array->arr[array->count++] = value;
    return STATUS_SUCCESS;
}

VOID QArrayRemove(IN PQWORD_ARRAY array, IN QWORD value)
{
    for(QWORD i = 0; i < array->count; i++)
    {
        if(array->arr[i] == value)
        {
            for(QWORD j = i; j < array->count - 1; j++)
                array->arr[j] = array->arr[j + 1];
            array->count--;
            i--;
        }
    }
}

BOOL QArrayIsExists(IN PQWORD_ARRAY array, IN QWORD value)
{
    for(QWORD i = 0; i < array->count; i++)
        if(array->arr[i] == value)
            return TRUE;
    return FALSE;
}