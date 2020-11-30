#ifndef __STRING_H_
#define __STRING_H_

#include <types.h>

typedef struct _UNICODE_STRING
{
    PWCHAR data;
    QWORD length;
} UNICODE_STRING, *PUNICODE_STRING;

BOOL UnicodeStringEquals(IN PUNICODE_STRING str1, IN PUNICODE_STRING str2);
QWORD UnicodeStringHash(IN PUNICODE_STRING str);

#endif