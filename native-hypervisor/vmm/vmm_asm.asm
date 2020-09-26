%define EFER_MSR 0xC0000080
%define COM1 0x3F8
%define COM2 0x2F8
%define COM3 0x3E8
%define COM4 0x2E8

%macro OutputSerial 1
    mov dx, COM3
    mov al, %1
    out dx, al
%endmacro

[BITS 64]

SECTION .text

global VmmToVm
global HandleVmExit
global SetupCompleteBackToGuestState

SetupCompleteBackToGuestState:
    mov rax, 0x0000681c
    mov rbx, rsp
    vmwrite rax, rbx
    vmlaunch
    pushf
    pop rax
    ret ; an error occured

VmmToVm:
    OutputSerial 'H'
    ret

HandleVmExit:
    OutputSerial 'A'
    vmresume
    OutputSerial 'B' ; This should never be executed
.assertHandleVmExit:
    jmp .assertHandleVmExit