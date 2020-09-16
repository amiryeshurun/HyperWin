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

; <NX disabled><11 reserved><40 PDPT address><11 bits 0><writable & readable><valid>
%macro SetPageEntryAtAddress 2
	mov eax, %1 ; dest
    mov edx, %2 ; value
    shl edx, 12
    or edx, 3
    mov [eax], edx
    mov [eax+4], 0 ; little endian
%endmacro

%macro Set2MB-PageEntryAtAddress 2
	mov eax, %1 ; dest
    mov edx, %2 ; value
    shl edx, 12
    or edx, (3 | 1 << 7)
    mov [eax], edx
    mov [eax+4], 0 ; little endian
%endmacro

%macro SetCr3BasePhysicalAddress 1
	mov eax, %1
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
; 0x7500 - ivt
; 0x17000 - cr3 (PML4 base addr, one cell)
; 0x2800000 - stack (long mode)

extern Initialize

; multiboot2 starts on 32bit protected mode
[BITS 32]
SEGMENT .text

multiboot2_header_start:
    dd 0xE85250D6 ; magic field, DWORD
    dd 0          ; architecture - i386 protected mode, DWORD
    dd multiboot2_header_end - multiboot2_header_start ; header length, DWORD
    dd 0xF0000000000 - (0xE85250D6 + (multiboot2_header_end - multiboot2_header_start) + 0) ; checksum, DWORD
    multiboot2_address_tag_start:
        dw 2 ; type, WORD
        dw 0 ; flags, WORD
        dd multiboot2_address_tag_end - multiboot2_address_tag_start ; dize, DWORD
        dd CODE_BEGIN_ADDRESS
        dd -1 ; data segment is present to the end of the imgae
        dd 0  ; bss
    multiboot2_address_tag_end:
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
    SetPageEntryAtAddress 0x17000, 0x18000 ; saved space for creating a seperation of virtual and physical memory
    xor ebx, ebx
    xor edi, edi
    mov ecx, COMPUTER_MEM_SIZE
.set_page_entries:
    SetPageEntryAtAddress 0x18000 + ebx, 0x19000 + ebx
    add edi, 0x1000
    add ebx, 8
    loop .set_page_entries

    xor ebx, ebx
    mov ecx, 512 * COMPUTER_MEM_SIZE
.set_page_entries_2:
    Set2MB-PageEntryAtAddress 0x19000 + ebx, ebx * LARGE_PAGE_SIZE
    add ebx, 8
    loop .set_page_entries_2




    SetPageEntryAtAddress 0x17000, 0x17008 ; PML4[0] = PDPT[0]
    SetPageEntryAtAddress 0x17008, 0x0 ; PDPT[0] = PDT[0]

    ; At this point I am allowed to work with addresses from 0 to (0x40000000 - 1)

    ; Set gdt
    mov eax, 0x1000 ; gdt
    mov word [eax], 0xff ; limit
    ; left = high part, right = low part. For more information, read AMD64 developer manual volume 2
    MovQwordToAddressLittleEndian 0x1002, 0x0, 0x2000 ; gdt address
    MovQwordToAddressLittleEndian 0x2000, 0x0, 0x0 ; null descriptor - 0
    MovQwordToAddressLittleEndian 0x2008, 0x190400, 0x0 ; code - long mode - 8
    MovQwordToAddressLittleEndian 0x2010, 0x90400, 0x0 ; data - long mode - 16
    MovQwordToAddressLittleEndian 0x2018, 0xcf9a00, 0xffff ; code - 32 bit mode - 24
    MovQwordToAddressLittleEndian 0x2020, 0x9a00, 0xffff ; code - 16 bit mode - 32
    MovQwordToAddressLittleEndian 0x2028, 0xcf9200, 0xffff ; data - 32 bit mode - 40
    MovQwordToAddressLittleEndian 0x2030, 0x9200, 0xffff ; data - 16 bit mode - 48
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
    mov rax, 8
    mov cs, rax
    mov rax, 16
    mov ds, rax
    mov ss, rax

    mov rsp, 0x2800000
    call Initialize ; goodbye assembly, hello C! (not really... just for a short time)
    pushf
    push 24
    push REAL_MODE_CODE_START
    iretq

[BITS 32]
SetupSystemAndHandleControlToBios:
    ; define the interrupt vector for real mode
    mov ax, 40
    mov ss, ax
    mov ds, ax
    mov eax, IVT_ADDRESS ; ivt
    mov word [eax], 0xff ; limit
    MovQwordToAddressLittleEndian IVT_ADDRESS + 2, 0x0, 0x0 ; ivt address (0)
    jmp 32:(SetupSystemAndHandleControlToBiosEnd - SetupRealMode + REAL_MODE_CODE_START)

[BITS 16]
SetupRealMode:
    mov ax, 48
    mov ss, ax
    mov ds, ax
    mov eax, cr0
    and eax, ~(1 | (1 << 31)) ; Disable paging & PM
    mov cr0, eax
    mov eax, cr4
    and eax, ~(1 << 5)        ; Disable PAE
    mov cr4, eax
    mov ecx, EFER_MSR
    rdmsr ; Value is stored in EDX:EAX
    and eax, ~(1 << 8)        ; Disable long mode
    wrmsr
    jmp 0:(SetupSystemAndHandleControlToBiosEnd - HandleControlToBios + REAL_MODE_CODE_START)

HandleControlToBios:
    mov dl, [WINDOWS_DISK_INDEX]
    mov ax, 0
    mov cs, 0
    mov ds, 0
    mov es, 0
    mov ss, 0
    jmp 0:MBR_ADDRESS
HandleControlToBiosEnd:
SetupSystemAndHandleControlToBiosEnd: