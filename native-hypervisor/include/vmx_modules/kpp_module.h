#ifndef __KPP_MODULE_H_
#define __KPP_MODULE_H_

#include <types.h>

typedef struct _KPP_MODULE_EXTENSION
{
    BYTE_PTR ntoskrnl;
    BYTE_PTR win32k;
} KPP_MODULE_EXTENSION, *PKPP_MODULE_EXTENSION;

#endif