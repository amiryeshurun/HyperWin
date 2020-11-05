#ifndef __MAP_H_
#define __MAP_H_

#include <types.h>
#include <error_codes.h>
#include <utils/array.h>

typedef QWORD (*HASH_FUNC)(IN QWORD);

typedef struct _QWORD_MAP
{
    PQWORD_PAIRS_ARRAY keyArrays;
    QWORD size;
    QWORD innerSize;
    HASH_FUNC hash;
} QWORD_MAP, *PQWORD_MAP;

QWORD MapGet(IN PQWORD_MAP map, IN QWORD key);
VOID MapSet(IN PQWORD_MAP map, IN QWORD key, IN QWORD value);
QWORD MapSize(IN PQWORD_MAP map);
STATUS MapCreate(OUT PQWORD_MAP map, IN HASH_FUNC hasher, IN QWORD size);

#endif