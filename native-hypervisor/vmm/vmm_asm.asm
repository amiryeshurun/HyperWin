%define EFER_MSR 0xC0000080
%define MBR_ADDRESS 0x7c00
%define DAP_ADDRESS 0x4000
%define FIRST_SECTOR_DEST 0x3000
%define BOOTABLE_SIGNATURE 0x55aa
%define REAL_MODE_OUTPUT_BUFFER_ADDRESS 0x2200
%define REAL_MODE_CODE_START 0x4200
%define SAVED_STACK_ADDRESS 0x6000
%define IVT_ADDRESS 0x7500
%define E820_OUTPUT_ADDRESS 0x8600
%define E820_MAGIC 0x534D4150
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

VmmToVm:
    ret

HandleVmExit:
    OutputSerial 'A'
    vmresume
    OutputSerial 'B' ; This should never be executed