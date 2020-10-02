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

extern HandleVmExitEx

SetupCompleteBackToGuestState:
    mov rax, 0x0000681c
    vmwrite rax, rsp
    vmlaunch
    pushf
    pop rax
    ret ; An error occured

VmmToVm:
    OutputSerial 'H'
    ret

HandleVmExit:
    mov qword [fs:0], rax
    mov qword [fs:8], rbx
    mov qword [fs:16], rcx
    mov qword [fs:24], rdx
    mov qword [fs:32], rsi
    mov qword [fs:40], rdi
    mov qword [fs:48], rbp
    mov qword [fs:56], r8
    mov qword [fs:64], r9
    mov qword [fs:72], r10
    mov qword [fs:80], r11
    mov qword [fs:88], r12
    mov qword [fs:96], r13
    mov qword [fs:104], r14
    mov qword [fs:112], r15
    mov rax, 0x0000681c    ; Geust RSP
    vmread rbx, rax
    mov qword [fs:120], rbx
    mov rax, 0x0000681e    ; Geust RIP
    vmread rbx, rax
    mov qword [fs:128], rbx

    ; Run handler function
    call HandleVmExitEx

    ; Change guest registers according to current state
    mov rcx, qword [fs:16]
    mov rdx, qword [fs:24]
    mov rsi, qword [fs:32]
    mov rdi, qword [fs:40]
    mov rbp, qword [fs:48]
    mov r8, qword [fs:56]
    mov r9, qword [fs:64]
    mov r10, qword [fs:72]
    mov r11, qword [fs:80]
    mov r12, qword [fs:88]
    mov r13, qword [fs:96]
    mov r14, qword [fs:104]
    mov r15, qword [fs:112]

    mov rax, 0x0000681c    ; geust RSP
    mov rbx, [fs:120]
    vmwrite rax, rbx
    mov rax, 0x0000681e    ; geust RIP
    mov rbx, [fs:128]
    vmwrite rax, rbx

    mov rax, qword [fs:0]
    mov rbx, qword [fs:8]

    ; Back to guest
    vmresume

   ; This should never be executed
.assertHandleVmExit:
    OutputSerial 'F'
    jmp .assertHandleVmExit