#ifndef __INTRINSICS_H_
#define __INTRINSICS_H_

#include <types.h>

#define INLINE inline
#define CARRY_FLAG_MASK 1
#define ZERO_FLAG_MASK (1 << 6)

__attribute__((always_inline))
VOID INLINE __vmread(QWORD field, QWORD_PTR value)
{
    asm volatile("vmread %1,%0" : "=rm" (*value) : "r" (field));
}

__attribute__((always_inline))
VOID INLINE __vmwrite(QWORD field, QWORD value)
{
    asm volatile("vmwrite %1,%0" :: "r" (field), "r" (value));
}

__attribute__((always_inline))
QWORD INLINE __vmxon(QWORD vmxonPhysicalAddress)
{
    QWORD flags;
    asm volatile("vmxon %1; pushf; pop %0" : "=r" (flags) : "m" (vmxonPhysicalAddress));
    if(flags & CARRY_FLAG_MASK)
        return 1;
    if(flags & ZERO_FLAG_MASK)
        return 2;
    return 0;
}

__attribute__((always_inline))
VOID INLINE __vmxoff()
{
    asm volatile("vmxoff");
}

__attribute__((always_inline))
QWORD INLINE __vmclear(QWORD vmcsPhysicalAddress)
{
    QWORD flags;
    asm volatile("vmclear %1; pushf; pop %0" : "=r" (flags) : "m" (vmcsPhysicalAddress));
    if(flags & CARRY_FLAG_MASK)
        return 1;
    if(flags & ZERO_FLAG_MASK)
        return 2;
    return 0;
}

__attribute__((always_inline))
QWORD INLINE __vmptrld(QWORD vmcsPhysicalAddress)
{
    QWORD flags;
    asm volatile("vmptrld %1; pushf; pop %0" : "=r" (flags) : "m" (vmcsPhysicalAddress));
    if(flags & CARRY_FLAG_MASK)
        return 1;
    if(flags & ZERO_FLAG_MASK)
        return 2;
    return 0;
}

__attribute__((always_inline))
VOID INLINE __writemsr(QWORD field, QWORD value)
{
    asm volatile("wrmsr" :: "c" (field), "d" (value >> 32), "a"(value & 0xffffffffULL));
}

__attribute__((always_inline))
QWORD INLINE __readmsr(QWORD field)
{
    QWORD upperHalf = 0, lowerHalf = 0;
    asm volatile("rdmsr" : "=d" (upperHalf), "=a" (lowerHalf) : "c"(field));
    return (upperHalf << 32) | lowerHalf;
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
QWORD INLINE __readcr0()
{
    QWORD cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    return cr0;
}

__attribute__((always_inline))
QWORD INLINE __readcr2()
{
    QWORD cr2;
    asm volatile("mov %%cr2, %0" : "=r"(cr2));
    return cr2;
}

__attribute__((always_inline))
QWORD INLINE __readcr3()
{
    QWORD cr3;
    asm volatile("mov %%cr3, %0" : "=r"(cr3));
    return cr3;
}

__attribute__((always_inline))
QWORD INLINE __readcr4()
{
    QWORD cr4;
    asm volatile("mov %%cr4, %0" : "=r"(cr4));
    return cr4;
}

__attribute__((always_inline))
VOID INLINE __writecr0(QWORD cr0)
{
    asm volatile("mov %0, %%cr0" :: "r"(cr0));
}

__attribute__((always_inline))
VOID INLINE __writecr2(QWORD cr2)
{
    asm volatile("mov %0, %%cr2" :: "r"(cr2));
}

__attribute__((always_inline))
VOID INLINE __writecr3(QWORD cr3)
{
    asm volatile("mov %0, %%cr3" :: "r"(cr3));
}

__attribute__((always_inline))
VOID INLINE __writecr4(QWORD cr4)
{
    asm volatile("mov %0, %%cr4" :: "r"(cr4));
}

__attribute__((always_inline))
QWORD INLINE __readflags()
{
    QWORD flags;
    asm volatile("pushf; pop %0": "=r" (flags));
    return flags;
}


#endif