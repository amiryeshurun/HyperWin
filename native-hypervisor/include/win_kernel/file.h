#ifndef __FILE_H_
#define __FILE_H_

#include <types.h>
#include <utils/string.h>

enum
{
    FILE_HIDE_CONTENT = 1
};

typedef struct _HIDDEN_FILE_RULE
{
    UNICODE_STRING content;
    BYTE rule;
    UNICODE_STRING optional;
} HIDDEN_FILE_RULE, *PHIDDEN_FILE_RULE;

typedef struct _HIDDEN_FILES_DATA
{
    UNICODE_STRING filePath;
    HIDDEN_FILE_RULE rule;
} HIDDEN_FILES_DATA, *PHIDDEN_FILES_DATA;

#endif