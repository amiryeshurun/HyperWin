#ifndef __FILE_H_
#define __FILE_H_

#include <types.h>
#include <utils/string.h>

enum
{
    FILE_HIDE_CONTENT = 1
};

enum
{
    SCB_FCB_OFFSET = 0xa8
};

enum
{
    FCB_MFT_INDEX = 0x8
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

STATUS Translate_SCB_To_FCB(IN QWORD scb, OUT QWORD_PTR fcb);
STATUS Get_FCB_Field(IN QWORD fcb, IN QWORD field, OUT PVOID value);

#endif