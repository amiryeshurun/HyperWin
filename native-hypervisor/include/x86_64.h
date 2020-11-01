#ifndef __X86_64_H_
#define __X86_64_H_

#include <types.h>

/* Paging related data */
#define PAGE_SIZE 0x1000
#define LARGE_PAGE_SIZE 0x200000
#define LARGE_PAGE_MASK 0x1fffffULL
#define GB_PAGE_MASK 0x3fffffffULL

/* CR related data */
#define CR0_NE_ENABLED (1 << 5)
#define CR0_PG_ENABLED (1 << 31)
#define CR0_PM_ENABLED (1 << 0)

#define CR4_VMX_ENABLED (1 << 13)
#define CR4_OSXSAVE (1 << 18)
#define CR4_PAE_ENABLED (1 << 5)

#define EFER_LONG_MODE_ENABLED (1 << 8)
#define EFER_LONG_MODE_ACTIVE (1 << 10)

/* CPUID related data */
#define CPUID_XSAVE (1 << 26)
#define CPUID_OSXSAVE (1 << 27)
#define CPUID_XSTATE_LEAF 0xd
#define CPUID_2XAPIC_AVAILABLE (1 << 21)
#define CPUID_GET_READ_PIPE 0x40020020
#define CPUID_GET_WRITE_PIPE 0x40040040

/* Useful opcodes */
#define INT3_OPCODE 0xcc
#define NOP_OPCODE 0x90

/* Useful flags */
#define RFLAGS_CARRY (1 << 0)

/* Architectural data */
#define X86_MAX_INSTRUCTION_LEN 15

/* Interrupts Data */
#define INT_DIVIDE_ERROR         0
#define INT_DEBUG                1
#define INT_NMI                  2
#define INT_BREAKPOINT           3
#define INT_OVERFLOW             4
#define INT_BOUND                5
#define INT_INVALID_OPCODE       6
#define INT_DEVICE_NOT_AVAIL     7
#define INT_DOUBLE_FAULT         8
#define INT_COPROCESSOR          9
#define INT_INVALID_TSS         10
#define INT_SEGMENT             11
#define INT_STACK_FAULT         12
#define INT_GENERAL_PROTECT     13
#define INT_PAGE_FAULT          14
#define INT_X87_FPU             16
#define INT_ALIGNMENT           17
#define INT_MACHINE_CHECK       18
#define INT_SIMD_FLOAT          19
#define INT_VIRTUAL_EXCE        20
#define INT_SECURITY            31

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