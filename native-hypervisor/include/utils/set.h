#ifndef __SET_H_
#define __SET_H_

#include <utils/array.h>
#include <utils/map.h>

typedef struct _QWORD_SET
{
    PQWORD_ARRAY array;
    QWORD buckets;
    HASH_FUNC hasher;
} QWORD_SET, *PQWORD_SET;

STATUS SetInit(IN PQWORD_SET set, IN QWORD bucketsCount, IN HASH_FUNC hasher);
STATUS SetInsert(IN PQWORD_SET set, IN QWORD value);
BOOL IsInSet(IN PQWORD_SET set, IN QWORD value);
VOID SetRemove(IN PQWORD_SET set, IN QWORD value);

#endif