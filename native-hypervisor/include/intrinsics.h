#ifndef __INTRINSICS_H_
#define __INTRINSICS_H_

#include <types.h>

#define INLINE inline
#define CARRY_FLAG_MASK 1
#define ZERO_FLAG_MASK (1 << 6)

__attribute__((always_inline))
VOID INLINE __vmread(IN QWORD field, OUT QWORD_PTR value)
{
    asm volatile("vmread %1,%0" : "=rm" (*value) : "r" (field));
}

__attribute__((always_inline))
VOID INLINE __vmwrite(IN QWORD field, IN QWORD value)
{
    asm volatile("vmwrite %1,%0" :: "r" (field), "r" (value));
}

__attribute__((always_inline))
QWORD INLINE __vmxon(IN QWORD vmxonPhysicalAddress)
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
QWORD INLINE __vmclear(IN QWORD vmcsPhysicalAddress)
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
QWORD INLINE __vmptrld(IN QWORD vmcsPhysicalAddress)
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
VOID INLINE __writemsr(IN QWORD field, IN QWORD value)
{
    asm volatile("wrmsr" :: "c" (field), "d" (value >> 32), "a"(value & 0xffffffffULL));
}

__attribute__((always_inline))
QWORD INLINE __readmsr(IN QWORD field)
{
    QWORD upperHalf = 0, lowerHalf = 0;
    asm volatile("rdmsr" : "=d" (upperHalf), "=a" (lowerHalf) : "c"(field));
    return (upperHalf << 32) | lowerHalf;
}

__attribute__((always_inline))
VOID INLINE __movsb(IN BYTE_PTR dest, IN BYTE_PTR src, IN QWORD length)
{
    asm volatile("rep; movsb" : [source] "=S" (src), [destination] "=D" (dest), [count] "=c" (length) : 
        "[source]" (src), "[destination]" (dest), "[count]" (length));
}

__attribute__((always_inline))
VOID INLINE __outbyte(IN DWORD port, IN BYTE data)
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
VOID INLINE __writecr0(IN QWORD cr0)
{
    asm volatile("mov %0, %%cr0" :: "r"(cr0));
}

__attribute__((always_inline))
VOID INLINE __writecr2(IN QWORD cr2)
{
    asm volatile("mov %0, %%cr2" :: "r"(cr2));
}

__attribute__((always_inline))
VOID INLINE __writecr3(IN QWORD cr3)
{
    asm volatile("mov %0, %%cr3" :: "r"(cr3));
}

__attribute__((always_inline))
VOID INLINE __writecr4(IN QWORD cr4)
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

__attribute__((always_inline))
VOID INLINE __writedr7(IN QWORD dr7)
{
    asm volatile("mov %0, %%dr7" :: "r"(dr7));
}

__attribute__((always_inline))
QWORD INLINE __readdr7()
{
    QWORD dr7;
    asm volatile("mov %%dr7, %0" : "=r"(dr7));
    return dr7;
}

__attribute__((always_inline))
VOID INLINE __cpuid(IN QWORD eax, IN QWORD ecx, OUT QWORD_PTR eaxOut, OUT QWORD_PTR ebxOut, OUT QWORD_PTR ecxOut, OUT QWORD_PTR edxOut)
{
    asm volatile ("cpuid" : "=a"(*eaxOut), "=b"(*ebxOut), "=c"(*ecxOut), "=d"(*edxOut)
        : "a"(eax), "c"(ecx));
}

__attribute__((always_inline))
VOID INLINE __xsetbv(IN DWORD edx, IN DWORD eax, IN DWORD ecx)
{
    asm volatile("xsetbv" :: "d"(edx), "a"(eax), "c"(ecx));
}

__attribute__((always_inline))
VOID INLINE __vmovdqu_ymm0(IN BYTE_PTR addr)
{
    asm volatile("vmovdqu %0, %%ymm0" :: "m" (addr));
}

__attribute__((always_inline))
VOID INLINE __movups_xmm0(IN BYTE_PTR addr)
{
    asm volatile("movups %0, %%xmm0" :: "m" (addr));
}

#endif