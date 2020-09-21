[BITS 64]

SEGMENT .text

global UpdateInstructionPointer

UpdateInstructionPointer: ; 1st = RIP offset
    add [rsp], rdi ; current return address is stored on the stack
    ret
