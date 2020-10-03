#ifndef __X86_64_H_
#define __X86_64_H_

#include <types.h>

/* Paging related data */
#define PAGE_SIZE 0x1000
#define LARGE_PAGE_SIZE 0x200000
#define LARGE_PAGE_MASK 0x1FFFFFULL

/* CR related data */
#define CR4_VMX_ENABLED (1 << 13)
#define CR0_NE_ENABLED (1 << 5)
#define CR4_OSXSAVE (1 << 18)

/* CPUID related data */
#define CPUID_XSAVE (1 << 26)

typedef struct _GDT
{
    WORD limit;
    QWORD address;
} __attribute__((__packed__)) GDT, *PGDT;

typedef struct _IDT
{
    WORD limit;
    QWORD address;
} __attribute__((__packed__)) IDT, *PIDT;

extern QWORD UpdateInstructionPointer(QWORD offset);
extern VOID GetGDT(PGDT address);
extern VOID GetIDT(PIDT address);
extern QWORD GetCS();
extern QWORD GetDS();
extern QWORD GetSS();
extern QWORD GetES();
extern QWORD GetFS();
extern QWORD GetGS();

#endif