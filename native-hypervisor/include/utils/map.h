#ifndef __MAP_H_
#define __MAP_H_

#include <types.h>
#include <error_codes.h>
#include <utils/array.h>
#include <utils/utils.h>

#define MAP_KEY_NOT_FOUND INF

typedef QWORD (*HASH_FUNC)(IN QWORD);
typedef BOOL (*EQUALITY_FUNC)(IN QWORD, IN QWORD);

typedef struct _QWORD_MAP
{
    PQWORD_PAIRS_ARRAY keyArrays;
    QWORD size;
    QWORD innerSize;
    HASH_FUNC hash;
    EQUALITY_FUNC equals;
} QWORD_MAP, *PQWORD_MAP;

QWORD MapGet(IN PQWORD_MAP map, IN QWORD key);
VOID MapSet(IN PQWORD_MAP map, IN QWORD key, IN QWORD value);
QWORD MapSize(IN PQWORD_MAP map);
STATUS MapCreate(OUT PQWORD_MAP map, IN HASH_FUNC hasher, IN QWORD size, IN EQUALITY_FUNC equals);
VOID MapGetValues(IN PQWORD_MAP map, OUT QWORD_PTR values, OUT QWORD_PTR count);
QWORD BasicHashFunction(IN QWORD key);
BOOL DefaultEqualityFunction(IN QWORD val1, IN QWORD val2);

#endif