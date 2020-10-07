%define EFER_MSR 0xC0000080
%define MBR_ADDRESS 0x7c00
%define DAP_ADDRESS 0x4000
%define SLEEP_TIME_FIRST_2 0x4000
%define SLEEP_TIME_SECOND_2 0x4002
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

%macro SetCr3BasePhysicalAddress 1
	mov eax, %1
    mov cr3, eax
%endmacro

%macro MovQwordToAddressLittleEndian 3
    mov eax, %1
    mov dword [eax], %3
    mov dword [eax+4], %2
%endmacro

%macro OutputSerial 1
    mov dx, COM3
    mov al, %1
    out dx, al
%endmacro

global DiskReader
global DiskReaderEnd
global AsmEnterRealModeRunFunction
global AsmEnterRealModeRunFunctionEnd
global EnterRealMode
global EnterRealModeEnd
global GetMemoryMap
global GetMemoryMapEnd
global SleepAsm
global SleepAsmEnd

SEGMENT .text

[BITS 64]
AsmEnterRealModeRunFunction:
    push rbp
    push rsp
    mov qword [SAVED_STACK_ADDRESS], rsp
    push 16
	mov rax, 0x25ff0
	push rax
	pushf
	push 24
	push REAL_MODE_CODE_START
	iretq   ; jmp with selector is not supported in long mode, use IRET instead
AsmReturnFromRealModeFunction:
    cli
    mov ax, 16
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov rsp, qword [SAVED_STACK_ADDRESS]
    pop rsp
    pop rbp
    ret

[BITS 32]
EnterRealMode:
    cli
    ; define the interrupt vector for real mode
    mov eax, IVT_ADDRESS ; ivt
    mov word [eax], 0xff ; limit
    mov dword [eax + 2], 0x0 ; ivt address (0)
    mov dword [eax + 6], 0x0
    lidt [IVT_ADDRESS]
    jmp 32:(DisableLongMode - EnterRealMode + REAL_MODE_CODE_START) ; 16 bit code selector

[BITS 16]
DisableLongMode:
    mov ax, 40
    mov ss, ax
    mov es, ax
    mov ds, ax
    mov fs, ax
    mov gs, ax

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
    mov ax, 0
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    mov eax, cr0
    or eax, 1
    mov cr0, eax ; the system is now in the same state as it was at boot time
    ; Enable long mode when back from handling interrupts
    jmp 24:(EnableProtectedMode - EnterRealMode + REAL_MODE_CODE_START)

[BITS 32]
EnableProtectedMode:
    cli
    mov ax, 16
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov gs, ax
    mov fs, ax
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
    mov ax, 0
    mov ss, ax
    mov es, ax
    mov ds, ax
    mov fs, ax
    mov gs, ax
    
    xor eax, eax
    mov si, DAP_ADDRESS
    mov ah, 0x42
    mov dl, byte [DAP_ADDRESS + 0x10]
    xor bx, bx
    int 13h
    jmp 0:(BackToLongMode - EnterRealMode + REAL_MODE_CODE_START)
DiskReaderEnd:

[BITS 16]
GetMemoryMap:
    mov ax, 0
    mov ss, ax
    mov es, ax
    mov ds, ax
    mov fs, ax
    mov gs, ax
    mov di, E820_OUTPUT_ADDRESS + 2
	xor ebx, ebx
	xor bp, bp
	mov edx, E820_MAGIC
	mov eax, 0xE820
	mov dword [es:di + 20], 1
	mov ecx, 24
	int 0x15
	jc short .failure
	mov edx, E820_MAGIC
	cmp eax, edx		
	jne short .failure
	test ebx, ebx
	je short .failure
	jmp short .jmpin
.e820lp:
	mov eax, 0xE820
	mov dword [es:di + 20], 1
	mov ecx, 24
	int 0x15
	jc short .e820f
	mov edx, E820_MAGIC
.jmpin:
	jcxz .skipent
	cmp cl, 20
	jbe short .notext
	test byte [es:di + 20], 1
	je short .skipent
.notext:
	mov ecx, [es:di + 8]
	or ecx, [es:di + 12]
	jz .skipent
	inc bp
	add di, 24
.skipent:
	test ebx, ebx
	jne short .e820lp
.e820f:
	mov word [E820_OUTPUT_ADDRESS], bp
	clc
	jmp 0:(BackToLongMode - EnterRealMode + REAL_MODE_CODE_START)
.failure:
    mov eax, E820_OUTPUT_ADDRESS
    mov byte [eax], 0
    stc
    jmp 0:(BackToLongMode - EnterRealMode + REAL_MODE_CODE_START)
GetMemoryMapEnd:

SleepAsm:
    mov ax, 0
    mov ss, ax
    mov es, ax
    mov ds, ax
    mov fs, ax
    mov gs, ax
    mov al, 0
    mov ah, 86h
    mov cx, [SLEEP_TIME_FIRST_2]
    mov dx, [SLEEP_TIME_SECOND_2]
    ; SLEEP!
    int 0x15
    jmp 0:(BackToLongMode - EnterRealMode + REAL_MODE_CODE_START)
SleepAsmEnd: