#ifndef __FILE_H_
#define __FILE_H_

#include <types.h>
#include <utils/string.h>
#include <error_codes.h>
#include <vmx_modules/hooking_module.h>

#define FILE_PATH_MAX_LENGTH 256

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
    BYTE encoding;
    UNICODE_STRING optional;
} HIDDEN_FILE_RULE, *PHIDDEN_FILE_RULE;

enum
{
    ENCODING_TYPE_UTF_8 = 0x1,
    ENCODING_TYPE_UTF_16 = 0x2
};

STATUS FileTranslateScbToFcb(IN QWORD scb, OUT QWORD_PTR fcb);
STATUS FileGetFcbField(IN QWORD fcb, IN QWORD field, OUT PVOID value);
STATUS FileAddNewProtectedFile(IN HANDLE fileHandle, IN BYTE_PTR content, IN QWORD contentLength, 
    IN BYTE encodingType);
STATUS FileRemoveProtectedFile(IN HANDLE fileHandle);
STATUS FileGetRuleByIndex(IN QWORD fileIndex, OUT PHIDDEN_FILE_RULE* rule);
STATUS FileHandleRead(IN PHOOK_CONTEXT context);
STATUS FileHandleReadReturn(IN PHOOK_CONTEXT context);

#endif