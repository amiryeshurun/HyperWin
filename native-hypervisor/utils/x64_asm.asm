[BITS 64]

SEGMENT .text

global UpdateInstructionPointer
global GetGDT
global GetIDT
global GetCS
global GetDS
global GetSS
global GetES
global GetFS
global GetGS

UpdateInstructionPointer: ; 1st = RIP offset
    add [rsp], rdi ; current return address is stored on the stack
    ret

GetGDT:
    sgdt [rdi]
    ret

GetIDT:
    sidt [rdi]
    ret

GetCS:
    mov rax, cs
    ret

GetDS:
    mov rax, ds
    ret

GetSS:
    mov rax, ss
    ret

GetES:
    mov rax, es
    ret

GetFS:
    mov rax, fs
    ret

GetGS:
    mov rax, gs
    ret