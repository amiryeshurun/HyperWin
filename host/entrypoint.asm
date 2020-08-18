%define EFER_MSR 0xC0000080

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

; multiboot2 starts on 32bit protected mode
[BITS 32]
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
    or edx, (1 << 7) ; 1GB page
    mov [eax], edx    

    ; At this point I am allowed to work with addresses from 0 to (0x40000000 - 1)

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

; 64-bit code goes here
[BITS 64]
