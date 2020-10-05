#ifndef __UTILS_H_
#define __UTILS_H_

#include <types.h>

#define ALIGN_UP(x, y) (((x) + ((y) - 1)) & ~((y) - 1))
#define ALIGN_DOWN(x, y) ((x) & (~((y) - 1)))

#define FIELD_OFFSET(type, field) (&((type)(0)->field))

#define INF 0xffffffffffffffffULL
#define NEG_INF (-0xffff)

VOID CopyMemory(OUT BYTE_PTR dest, IN BYTE_PTR src, IN QWORD count);
INT CompareMemory(IN BYTE_PTR buff1, IN BYTE_PTR buff2, IN QWORD length);
CHAR ConvertHalfByteToHexChar(IN BYTE halfByte);
QWORD pow(IN QWORD base, IN QWORD power);
QWORD NumberOfDigits(IN QWORD number);
QWORD StringLength(IN PCHAR str);
VOID SetMemory(IN BYTE_PTR base, IN BYTE value, IN QWORD length);
VOID DumpHostStack(IN QWORD_PTR stackAddress);

#endif
