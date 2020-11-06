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

STATUS QPArrayInit(OUT PQWORD_PAIRS_ARRAY array);
STATUS QPArrayInsert(IN PQWORD_PAIRS_ARRAY array, IN PQWORD_PAIR value);

#endif