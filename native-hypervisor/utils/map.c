#include <utils/map.h>
#include <debug.h>
#include <vmm/vmm.h>
#include <utils/allocation.h>

QWORD MapGet(IN PQWORD_MAP map, IN QWORD key)
{
    QWORD hash = map->hash(key);
    if(map->arr[hash].entriesCount == 1)
        return map->arr[hash].entries[0].value;
    else
        for(QWORD i = 0; i < map->arr[hash].entriesCount; i++)
            if(map->arr[hash].entries[i].key == key)
                return map->arr[hash].entries[i].value;
    return INF;
}

VOID MapSet(IN PQWORD_MAP map, IN QWORD key, IN QWORD value)
{

}

QWORD MapSize(IN PQWORD_MAP map)
{
    return map->size;
}

STATUS MapCreate(OUT PQWORD_MAP map, IN HASH_FUNC hasher, IN QWORD size)
{
    return STATUS_SUCCESS;
}