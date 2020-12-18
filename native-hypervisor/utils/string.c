#include <utils/string.h>
#include <utils/utils.h>

BOOL UnicodeStringEquals(IN PUNICODE_STRING str1, IN PUNICODE_STRING str2)
{
    if(str1->length != str2->length)
        return FALSE;
    return !CompareMemory(str1->data, str2->data, str1->length);
}

QWORD UnicodeStringHash(IN PUNICODE_STRING str)
{
    QWORD hash = 5381;
    for(QWORD i = 0; i < str->length; i++)
        hash = ((hash << 5) + hash) + (BYTE_PTR)(str->data)[i];
    return hash % BASIC_HASH_LEN;
}