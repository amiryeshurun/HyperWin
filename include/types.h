#ifndef __TYPES_H_
#define __TYPES_H_

typedef unsigned int UINT32;
typedef unsigned long DWORD, *DWORD_PTR;
typedef unsigned long long UINT64;
typedef unsigned long long QWORD, *QWORD_PTR;
typedef unsigned char BOOL;
typedef unsigned char BYTE, *BYTE_PTR;
typedef unsigned short WORD, *WORD_PTR;
typedef UINT32 PTR;

#define VOID void
#define FALSE (0)
#define TRUE (!(FALSE))

#define OUT
#define IN

#ifndef NULL
#define NULL (void*)0
#endif

typedef struct _REGISTERS
{
    QWORD rax;
    QWORD rbx;
    QWORD rcx;
    QWORD rdx;
    QWORD rsi;
    QWORD rdi;
    QWORD rbp;
    QWORD rsp;
    QWORD r8;
    QWORD r9;
    QWORD r10;
    QWORD r11;
    QWORD r12;
    QWORD r13;
    QWORD r14;
    QWORD r15;
}REGISTERS, *PREGISTERS;

#endif