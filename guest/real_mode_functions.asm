%define EFER_MSR 0xC0000080
%define MBR_ADDRESS 0x7c00
%define DAP_ADDRESS 0x4000
%define FIRST_SECTOR_DEST 0x3000
%define BOOTABLE_SIGNATURE 0x55aa
%define REAL_MODE_OUTPUT_BUFFER_ADDRESS 0x2200
%define REAL_MODE_CODE_START 0x4200
%define SAVED_STACK_ADDRESS 0x6000
%define IVT_ADDRESS 0x7500


%macro SetCr3BasePhysicalAddress 1
	mov eax, %1
    shl eax, 11
    mov cr3, eax ; need to test - not sure about it, what happens to the 32 MSBs?
%endmacro

global DiskReader
global AsmEnterRealModeRunFunction

SEGMENT .text

[BITS 64]
AsmEnterRealModeRunFunction:
    push rbp
    push rsp
    mov [SAVED_STACK_ADDRESS], rsp
    pushf
    push 24
    push REAL_MODE_CODE_START
    iretq
AsmReturnFromRealModeFunction:
    cli
    mov rax, 8
    mov cs, rax
    mov rax 16
    mov ds, rax
    mov ss, rax
    mov rsp, [SAVED_STACK_ADDRESS]
    pop rsp
    pop rbp
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
    mov ax, 48
    mov ds, ax
    mov ss, ax
    ; Disable long mode here, then execute desired function
    mov eax, cr0
    and eax, ~(1 | (1 << 31)) ; Disable paging & PM
    mov cr0, eax
    mov eax, cr4
    and eax, ~(1 << 5)
    mov cr4, eax
    mov ecx, EFER_MSR
    rdmsr ; Value is stored in EDX:EAX
    and eax, ~(1 << 8)
    wrmsr
    jmp 0:(EnterRealModeEnd - EnterRealMode + REAL_MODE_CODE_START)

BackToLongMode:
    cli
    lgdt [0x1000]
    mov eax, cr0
    or eax, (1 << 0)
    mov cr0, eax ; the system is now in the same state as it was at boot time
    mov ax, 0
    mov ss, ax
    mov ds, ax
    ; Enable long mode when back from handling interrupts
    jmp 24:(EnableProtectedMode - EnterRealMode + REAL_MODE_CODE_START)

[BITS 32]
EnableProtectedMode:
    mov eax, cr0
    and eax, ~(1 << 31)
    mov cr0, eax
    mov eax, cr4
    or eax, (1 << 5)
    mov cr4, eax
    SetCr3BasePhysicalAddress 0x17000 ; set cr3 with PML4[0]
    mov ecx, EFER_MSR
    rdmsr ; Value is stored in EDX:EAX
    or eax, (1 << 8)
    wrmsr
    mov eax, cr0
    or eax, (1 << 31)
    mov cr0, eax
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