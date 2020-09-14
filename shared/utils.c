#include <utils.h>
#include <intrinsics.h>

VOID CopyMemory(OUT QWORD_PTR dest, IN QWORD_PTR src, IN QWORD count)
{
    __movsb((BYTE_PTR)dest, (BYTE_PTR)src, count);
}