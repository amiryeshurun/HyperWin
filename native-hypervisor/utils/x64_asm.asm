[BITS 64]

SEGMENT .text

global UpdateInstructionPointer
global GetGDT
global GetIDT

UpdateInstructionPointer: ; 1st = RIP offset
    add [rsp], rdi ; current return address is stored on the stack
    ret

GetGDT:
    sgdt [rdi]
    ret

GetIDT:
    sidt [rdi]
    ret