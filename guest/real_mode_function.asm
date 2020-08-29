%define EFER_MSR 0xC0000080
%define MBR_ADDRESS 0x7c00
%define DAP_ADDRESS 0x4000
%define FIRST_SECTOR_DEST 0x3000
%define BOOTABLE_SIGNATURE 0x55aa
%define REAL_MODE_OUTPUT_BUFFER_ADDRESS 0x2200
%define REAL_MODE_CODE_START 0x4200
%define SAVED_STACK_ADDRESS 0x6000
%define IVT_ADDRESS 0x7500

global DiskReader
global AsmEnterRealModeRunFunction

[BITS 64]
AsmEnterRealModeRunFunction:
    mov [SAVED_STACK_ADDRESS], rsp
    pushf
    push 24
    iretq
AsmReturnFromRealModeFunction:
    cli
    mov cs, 8
    mov ds, 16
    mov ss, 16
    mov rsp, [SAVED_STACK_ADDRESS]
    ret

[BITS 32]
EnterRealMode:
    ; define the interrupt vector for real mode
    mov eax, IVT_ADDRESS ; ivt
    mov word [eax], 0xff ; limit
    MovQwordToAddressLittleEndian IVT_ADDRESS + 2, 0x0, 0x0 ; ivt address (0)
    lidt [IVT_ADDRESS]
    jmp 32:(DisableLongMode - EnterRealMode + REAL_MODE_CODE_START) ; 16 bit code selector

[BITS 16]
DisableLongMode:
    ; Disable long mode here, execute desired function
    jmp 0:(EnterRealModeEnd - EnterRealMode + REAL_MODE_CODE_START)

BackToLongMode:
    ; Enable long mode when back from handling interrupts
    jmp 24:(EnableProtectedMode - EnterRealMode + REAL_MODE_CODE_START)

[BITS 32]
EnableProtectedMode:
    jmp 8:AsmReturnFromRealModeFunction

EnterRealModeEnd:

[BITS 16]
DiskReader:
    mov si, DAP_ADDRESS
    mov ah, 0x42
    mov dl, [DAP_ADDRESS + 0x10]
    xor bx, bx
    int 13h
    jmp 0:(BackToLongMode - EnterRealMode + REAL_MODE_CODE_START)
DiskReaderEnd: