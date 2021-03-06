#include <utils/map.h>
#include <debug.h>
#include <vmm/vmm.h>
#include <utils/allocation.h>

QWORD BasicHashFunction(IN QWORD key)
{
    return SumDigits(key) % BASIC_HASH_LEN;
}

QWORD MapGet(IN PQWORD_MAP map, IN QWORD key)
{
    QWORD hash;
    hash = map->hash(key);
    if(map->keyArrays[hash].count == 1 && map->equals(map->keyArrays[hash].arr[0]->key, key))
        return map->keyArrays[hash].arr[0]->value;
    else
        for(QWORD i = 0; i < map->keyArrays[hash].count; i++)
            if(map->equals(map->keyArrays[hash].arr[i]->key, key))
                return map->keyArrays[hash].arr[i]->value;
    return MAP_KEY_NOT_FOUND;
}

VOID MapOverride(IN PQWORD_MAP map, IN QWORD key, IN QWORD value)
{
    QWORD hash;

    hash = map->hash(key);
    if(map->keyArrays[hash].count == 1 && map->equals(map->keyArrays[hash].arr[0]->key, key))
        map->keyArrays[hash].arr[0]->value = value;
    else
    {
        for(QWORD i = 0; i < map->keyArrays[hash].size; i++)
        {
            if(map->equals(map->keyArrays[hash].arr[i]->key, key))
            {
                map->keyArrays[hash].arr[i]->value = value;
                return;
            }
        }
    }
}

QWORD MapRemove(IN PQWORD_MAP map, IN QWORD key)
{
    QWORD hash, ret;

    hash = map->hash(key);
    ret = QPArrayRemove(&map->keyArrays[hash], key);
    
    return ret == VALUE_NOT_FOUND ? MAP_KEY_NOT_FOUND : ret;
}

VOID MapSet(IN PQWORD_MAP map, IN QWORD key, IN QWORD value)
{
    PHEAP heap;
    PQWORD_PAIR pair;
    QWORD hash;

    heap = &VmmGetVmmStruct()->currentCPU->sharedData->heap;
    hash = map->hash(key);
    // Check if the key already exist
    if(MapGet(map, key) == MAP_KEY_NOT_FOUND)
    {
        if(heap->allocate(heap, sizeof(QWORD_PAIR), &pair) != STATUS_SUCCESS)
        {
            Print("Count not allocate space for map\n");
            ASSERT(FALSE);
        }
        pair->key = key;
        pair->value = value;
        map->size++;
        ASSERT(QPArrayInsert(&(map->keyArrays[hash]), pair) == STATUS_SUCCESS);
    }
    else
        MapOverride(map, key, value);
}

QWORD MapSize(IN PQWORD_MAP map)
{
    return map->size;
}

VOID MapGetValues(IN PQWORD_MAP map, OUT QWORD_PTR values, OUT QWORD_PTR count)
{
    *count = 0;
    for(QWORD i = 0; i < map->innerSize; i++)
        for(QWORD j = 0; j < map->keyArrays[i].count; j++)
            values[(*count)++] = map->keyArrays[i].arr[j]->value;
}

STATUS MapCreate(OUT PQWORD_MAP map, IN HASH_FUNC hasher, IN QWORD size, IN EQUALITY_FUNC equals)
{
    PHEAP heap;

    heap = &(VmmGetVmmStruct()->currentCPU->sharedData->heap);
    map->hash = hasher;
    map->innerSize = size;
    map->equals = equals;
    if(heap->allocate(heap, size * sizeof(QWORD_PAIRS_ARRAY), &map->keyArrays) != STATUS_SUCCESS)
    {
        Print("Could not initialize map\n");
        return STATUS_NO_MEM_AVAILABLE;
    }
    HwSetMemory(map->keyArrays, 0, size * sizeof(QWORD_PAIRS_ARRAY));
    for(QWORD i = 0; i < size; i++)
        QPArrayInit(&(map->keyArrays[i]));
    map->size = 0;
    return STATUS_SUCCESS;
}

BOOL DefaultEqualityFunction(IN QWORD val1, IN QWORD val2)
{
    return val1 == val2;
}