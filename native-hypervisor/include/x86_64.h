#ifndef __X86_H_
#define __X86_H_

#include <types.h>

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

extern VOID UpdateInstructionPointer(QWORD offset);
extern VOID GetGDT(PGDT address);
extern VOID GetIDT(PIDT address);

#endif