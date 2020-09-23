%define EFER_MSR 0xC0000080
%define MBR_ADDRESS 0x7c00
%define DAP_ADDRESS 0x4000
%define FIRST_SECTOR_DEST 0x3000
%define BOOTABLE_SIGNATURE 0x55aa
%define REAL_MODE_OUTPUT_BUFFER_ADDRESS 0x2200
%define REAL_MODE_CODE_START 0x4200
%define WINDOWS_DISK_INDEX 0x6010
%define CODE_BEGIN_ADDRESS 0x120000
%define COMPUTER_MEM_SIZE 16
%define LARGE_PAGE_SIZE 0x200000
%define IVT_ADDRESS 0x7500
%define COM1 0x3F8
%define COM2 0x2F8
%define COM3 0x3E8
%define COM4 0x2E8

; <NX disabled><11 reserved><40 PDPT address><11 bits 0><writable & readable><valid>
%macro SetPageEntryAtAddress 2
	mov eax, %1 ; dest
    mov edx, %2 ; value
    shl edx, 12
    or edx, 3
    mov dword [eax], edx
    mov dword [eax+4], 0 ; little endian
%endmacro

%macro Set2MBPageEntryAtAddress 2
	mov eax, %1 ; dest
    mov edx, %2 ; value
    shl edx, 12
    or edx, (3 | 1 << 7)
    mov dword [eax], edx
    mov dword [eax+4], 0 ; little endian
%endmacro

%macro SetCr3BasePhysicalAddress 1
	mov eax, %1
    mov cr3, eax ; need to test - not sure about it, what happens to the 32 MSBs?
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

; 0x1000, 0x2000 - gdt
; 0x2200 - outout buffer for real mode functions
; 0x3000 - first sector dest
; 0x4000 - DAP
; 0x4200 - real mode code start
; 0x7500 - ivt
; 0x17000 - cr3 (PML4 base addr)
; 0x2800000 - stack (long mode)

extern Initialize

global _start
global SetupSystemAndHandleControlToBios
global SetupSystemAndHandleControlToBiosEnd

; multiboot2 starts on 32bit protected mode
[BITS 32]
SEGMENT .text

multiboot2_header_start:
    dd 0xE85250D6 ; magic field, DWORD
    dd 0          ; architecture - i386 protected mode, DWORD
    dd multiboot2_header_end - multiboot2_header_start ; header length, DWORD
    dd 0x100000000 - (0xE85250D6 + (multiboot2_header_end - multiboot2_header_start) + 0) ; checksum, DWORD
    multiboot2_address_tag_start:
        dw 2 ; type, WORD
        dw 0 ; flags, WORD
        dd multiboot2_address_tag_end - multiboot2_address_tag_start ; dize, DWORD
        dd CODE_BEGIN_ADDRESS
        dd -1 ; data segment is present to the end of the imgae
        dd 0  ; bss
        dd 0
    multiboot2_address_tag_end:
    multiboot2_entry_address_tag_start:
        dw 3      ; type, WORD
        dw 0      ; flags, WORD
        dd multiboot2_entry_address_tag_end - multiboot2_entry_address_tag_start ; size, DWORD
        dd _start ; entrypoint, DWORD
    multiboot2_entry_address_tag_end:
        dd 0
        dd 0
        dw 8
multiboot2_header_end:

_start:
    ; Create a "linear address" page table. This is usefull because it is much easier to reffer to "physical"
    ; addresses in order to load the MBR
    MovQwordToAddressLittleEndian 0x17000, 0x0, 0x18003
    mov eax, 0x19000
    mov edi, 0x18000
    mov ecx, COMPUTER_MEM_SIZE
.setup_pdpt:
    mov edx, eax
    or edx, 3
    mov dword [edi], edx
    mov dword [edi + 4], 0
    add eax, 0x1000
    add edi, 8
    loop .setup_pdpt

    mov ecx, 3                  ; map 3GB
    shl ecx, 9                  ; multiply by 512
    xor eax, eax                ; start address is 0
    mov ebx, ((1 << 7) | (1 << 1) | 1)
    mov edi, 0x19000
.setup_pds:
    or eax, ebx
    mov dword [edi], eax
    mov dword [edi + 4], 0
    add edi, 8
    add eax, LARGE_PAGE_SIZE
    loop .setup_pds
    ; At this point I am allowed to work with addresses from 0 to 3GB

    ; Set gdt
    mov eax, 0x1000 ; gdt
    mov word [eax], 0x00ff ; limit
    MovQwordToAddressLittleEndian 0x1002, 0x0, 0x2000
    ; left = high part, right = low part. For more information, read AMD64 developer manual volume 2, GDT
    MovQwordToAddressLittleEndian 0x2000, 0x0, 0x0 ; null descriptor - 0
    MovQwordToAddressLittleEndian 0x2008, 0x209a00, 0x0	; code - long mode - 8
    MovQwordToAddressLittleEndian 0x2010, 0xcf9200, 0xffff ; data - long and compatibility - 16
    MovQwordToAddressLittleEndian 0x2018, 0xcf9a00, 0xffff ; code - 32-bit mode - 24
    MovQwordToAddressLittleEndian 0x2020, 0x9a00, 0xffff ; code - 16 bit (not real) - 32
    MovQwordToAddressLittleEndian 0x2028, 0x9200, 0xffff ; data - 16 bit (not real) - 40
    lgdt [0x1000]

    ; Enter long mode - see docs/host/entrypoint.md for details
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
    ; The CPU is now in compatibility mode.
    ; Still need to load the GDT with the 64-bit flags set in the code and data selectors.

    jmp 8:CompatibilityTo64

; 64-bit code goes here
[BITS 64]
CompatibilityTo64:
    cli
    mov ax, 16    
    mov ss, ax
    mov es, ax
    mov ds, ax
    mov fs, ax
    mov gs, ax

    mov rcx, COMPUTER_MEM_SIZE  ; map ALL available memory
    shl rcx, 9                  ; multiply by 512
    xor rax, rax                ; start address is 0
    or rax, ((1 << 7) | (1 << 1) | 1)
    mov rdi, 0x19000
.setup_pds_long_mode:
    mov qword [rdi], rax
    add rdi, 8
    add rax, LARGE_PAGE_SIZE
    loop .setup_pds_long_mode

    mov rsp, 0x2800000
    call Initialize ; goodbye assembly, hello C! (not really... just for a short time)
    push 16
    mov rax, 0x25ff0
    push rax
    pushf
    push 24 ; 32 bit code selector
    push REAL_MODE_CODE_START
    iretq

[BITS 32]
SetupSystemAndHandleControlToBios:
    ; define the interrupt vector for real mode
    cli
    mov eax, IVT_ADDRESS ; ivt
    mov word [eax], 0xff ; limit
    mov dword [eax + 2], 0x0 ; ivt address (0)
    mov dword [eax + 6], 0x0
    lidt [IVT_ADDRESS]
    jmp 32:(SetupRealMode - SetupSystemAndHandleControlToBios + REAL_MODE_CODE_START) ; 16 bit code selector

[BITS 16]
SetupRealMode:
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
    jmp 0:(HandleControlToBios - SetupSystemAndHandleControlToBios + REAL_MODE_CODE_START)

HandleControlToBios:
    mov dl, [WINDOWS_DISK_INDEX]
    mov ax, 0
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax

    jmp 0:MBR_ADDRESS
HandleControlToBiosEnd:
SetupSystemAndHandleControlToBiosEnd:
