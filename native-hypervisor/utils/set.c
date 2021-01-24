#include <utils/set.h>
#include <vmm/vmm.h>
#include <utils/allocation.h>
#include <debug.h>

STATUS SetInit(IN PQWORD_SET set, IN QWORD bucketsCount, IN HASH_FUNC hasher)
{
    PHEAP heap;
    STATUS status = STATUS_SUCCESS;

    heap = &VmmGetVmmStruct()->currentCPU->sharedData->heap;
    SUCCESS_OR_CLEANUP(heap->allocate(heap, bucketsCount * sizeof(QWORD_ARRAY), &set->array));
    for(QWORD i = 0; i < bucketsCount; i++)
        QArrayInit(&set->array[i]);
    set->hasher = hasher;
    set->buckets = bucketsCount;

cleanup:
    return status;
}

STATUS SetInsert(IN PQWORD_SET set, IN QWORD value)
{
    QWORD hash;
    hash = set->hasher(value);
    if(IsInSet(set, value))
        return STATUS_KEY_ALREADY_EXISTS;
    return QArrayInsert(&(set->array[hash]), value);
}

BOOL IsInSet(IN PQWORD_SET set, IN QWORD value)
{
    return QArrayIsExists(&(set->array[set->hasher(value)]), value);
}

VOID SetRemove(IN PQWORD_SET set, IN QWORD value)
{
    QArrayRemove(&(set->array[set->hasher(value)]), value);
}