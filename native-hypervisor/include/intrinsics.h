#ifndef __INTRINSICS_H_
#define __INTRINSICS_H_

#include <types.h>
#define INLINE inline

__attribute__((always_inline))
VOID INLINE __vmread(QWORD field, QWORD_PTR value)
{
    asm volatile("vmread %1,%0" : "=rm" (*value) : "r" (field));
}

__attribute__((always_inline))
VOID INLINE __movsb(BYTE_PTR dest, BYTE_PTR src, QWORD length)
{
    asm volatile("rep; movsb" : [source] "=S" (src), [destination] "=D" (dest), [count] "=c" (length) : 
        "[source]" (src), "[destination]" (dest), "[count]" (length));
}

__attribute__((always_inline))
VOID INLINE __outbyte(DWORD port, BYTE data)
{
    asm volatile("out %1, %0" :: "d" (port), "a" (data));
}

__attribute__((always_inline))
QWORD INLINE __readcr3()
{
    QWORD cr3;
    asm volatile("mov %%cr3, %0" : "=r"(cr3));
    return cr3;
}

#endif