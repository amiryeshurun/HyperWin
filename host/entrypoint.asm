%define EFER_MSR 0xC0000080
%define MBR_ADDRESS 0x7c00
%define DAP_ADDRESS 0x4000
%define FIRST_SECTOR_DEST 0x3000
%define BOOTABLE_SIGNATURE 0x55aa
%define REAL_MODE_OUTPUT_BUFFER_ADDRESS 0x2200
%define REAL_MODE_CODE_START 0x4200

; <NX disabled><11 reserved><40 PDPT address><11 bits 0><writable & readable><valid>
%macro SetPageEntryAtAddress 2
	mov eax, %1 ; dest
    mov edx, %2 ; value
    shl edx, 11
    or edx, 3
    mov [eax], edx
    mov [eax+4], 0 ; little endian
%endmacro

%macro SetCr3BasePhysicalAddress 1
	mov eax, %1
    shl eax, 11
    mov cr3, eax ; need to test - not sure about it, what happens to the 32 MSBs?
%endmacro

%macro MovQwordToAddressLittleEndian 3
    mov eax, %1
    mov [eax], %3
    mov [eax+4], %2
%endmacro

; 0x1000, 0x2000 - gdt
; 0x2200 - outout buffer for real mode functions
; 0x3000 - first sector dest
; 0x4000 - DAP
; 0x4200 - real mode code start
; 0x17000 - cr3 (PML4 base addr, one cell)
; 0x2800000 - stack (long mode)

extern Initialize

; multiboot2 starts on 32bit protected mode
[BITS 32]
SEGMENT .text

multiboot2_header_start:
    dd 0xE85250D6 ; magic field, DWORD
    dd 0          ; architecture - i386 safe mode, DWORD
    dd multiboot2_header_end - multiboot2_header_start ; header length, DWORD
    dd 0xF0000000000 - (0xE85250D6 + (multiboot2_header_end - multiboot2_header_start) + 0) ; checksum, DWORD
    multiboot2_entry_address_tag_start:
        dw 3      ; type, WORD
        dw 0      ; flags, WORD
        dd multiboot2_entry_address_tag_end - multiboot2_entry_address_tag_start ; size, DWORD
        dw _hypervisor_entrypoint ; entrypoint, DWORD
    multiboot2_entry_address_tag_end:
    multiboot2_end_tags_start:
        dd 0
        dd 0
        dw 8
    multiboot2_end_tags_end:
multiboot2_header_end:

_hypervisor_entrypoint:
    ; Create a "linear address" page table. This is usefull because it is much easier to reffer to "physical"
    ; addresses in order to load the MBR
    ; (cr3)PML4[0] = 0x17000
    ; (0x17000)PDPT[0] = 0x17008 - points to physical address 0
    ; (0) = 1GB page for hypervisor initialization (starting from physical address 0)

    SetPageEntryAtAddress 0x17000, 0x17008 ; PML4[0] = PDPT[0]
    SetPageEntryAtAddress 0x17008, 0x0 ; PDPT[0] = PDT[0]
    mov eax, 0x17008
    mov edx, [eax]
    or edx, (1 << 7); 1GB page
    mov [eax], edx    
    ; At this point I am allowed to work with addresses from 0 to (0x40000000 - 1)

    ; Set gdt
    mov eax, 0x1000 ; gdt
    mov word [eax], 0xff ; limit
    ; left = high part, right = low part. For more information, read AMD64 developer manual volume 2
    MovQwordToAddressLittleEndian 0x1008, 0x0, 0x2000 ; gdt address
    MovQwordToAddressLittleEndian 0x2000, 0x0, 0x0 ; null descriptor
    MovQwordToAddressLittleEndian 0x2008, 0x190400, 0x0 ; code - long mode
    MovQwordToAddressLittleEndian 0x2010, 0x90400, 0x0 ; data - long mode
    MovQwordToAddressLittleEndian 0x2018, 0xcf9a00, 0xffff ; code - 32 bit mode
    MovQwordToAddressLittleEndian 0x2020, 0x9a00, 0xffff ; code - 16 bit mode
    MovQwordToAddressLittleEndian 0x2028, 0xcf9200, 0xffff ; data - 32 bit mode
    MovQwordToAddressLittleEndian 0x2030, 0x9200, 0xffff ; data - 16 bit mode
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
    mov cs, 8
    mov ds, 16
    mov ss, 16

    mov rsp, 0x2800000
    call Initialize ; goodbye assembly, hello C! (not really... just for a short time)

