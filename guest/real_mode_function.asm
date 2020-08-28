%define EFER_MSR 0xC0000080
%define MBR_ADDRESS 0x7c00
%define DAP_ADDRESS 0x4000
%define FIRST_SECTOR_DEST 0x3000
%define BOOTABLE_SIGNATURE 0x55aa
%define REAL_MODE_OUTPUT_BUFFER_ADDRESS 0x2200
%define REAL_MODE_CODE_START 0x4200
%define SavedStackAddress 0x6000

global DiskReader
global AsmEnterRealModeRunFunction

[BITS 64]
AsmEnterRealModeRunFunction:
    mov [SavedStackAddress], rsp
    pushf
    push 8
    iretq
AsmEnterLongModeFromFunction:
    cli
    mov cs, 8
    mov ds, 8
    mov rsp, [SavedStackAddress]

[BITS 32]
EnterRealMode:
    SetUpInterruptVector:
        ; define the interrupt vector for real mode
EnterRealModeEnd:

[BITS 16]
DiskReader:
    mov si, DAP_ADDRESS
    mov ah, 0x42
    mov dl, [DAP_ADDRESS + 0x10]
    xor bx, bx
    int 13h
DiskReaderEnd: