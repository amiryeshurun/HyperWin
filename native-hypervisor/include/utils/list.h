#ifndef __LIST_H_
#define __LIST_H_

#include <types.h>
#include <error_codes.h>

typedef struct _LIST_ENTRY
{
    struct _LIST_ENTRY* next;
    struct _LIST_ENTRY* prev;
    QWORD data;
} LIST_ENTRY, *PLIST_ENTRY;

typedef struct _LIST
{
    PLIST_ENTRY head;
    PLIST_ENTRY tail;
    QWORD size;
} LIST, *PLIST;

STATUS ListCreate(OUT PLIST list);
STATUS ListInsert(IN PLIST list, IN QWORD data);
STATUS ListRemove(IN PLIST list, IN QWORD data);
BOOL ListContains(IN PLIST lise, IN QWORD data);

#endif