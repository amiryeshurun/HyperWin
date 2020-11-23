#ifndef __ARRAY_H_
#define __ARRAY_H_

#include <types.h>
#include <error_codes.h>

typedef struct _QWORD_PAIRS_ARRAY
{
    PQWORD_PAIR* arr;
    QWORD size;
    QWORD count;
} QWORD_PAIRS_ARRAY, *PQWORD_PAIRS_ARRAY;

typedef struct _QWORD_ARRAY
{
    QWORD_PTR arr;
    QWORD size;
    QWORD count;
} QWORD_ARRAY, *PQWORD_ARRAY;

STATUS QPArrayInit(OUT PQWORD_PAIRS_ARRAY array);
STATUS QPArrayInsert(IN PQWORD_PAIRS_ARRAY array, IN PQWORD_PAIR value);

STATUS QArrayInit(OUT PQWORD_ARRAY array);
STATUS QArrayInsert(IN PQWORD_ARRAY array, IN QWORD value);
BOOL QArrayIsExists(IN PQWORD_ARRAY array, IN QWORD value);
VOID QArrayRemove(IN PQWORD_ARRAY array, IN QWORD value);

#endif