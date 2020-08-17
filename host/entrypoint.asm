%define EFER_MSR 0xC0000080

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
    ; Need to take care of paging configurations, still reading about it
    ; ...

    ; Enter long mode - see docs/host/entrypoint.md for details
    mov eax, cr0
    and eax, ~(1 << 31)
    mov cr0, eax
    mov eax, cr4
    or eax, (1 << 5)
    mov cr4, eax
    ; mov cr3, PML4 <---- in progress
    mov ecx, EFER_MSR
    rdmsr ; Value is stored in EDX:EAX
    or eax, (1 << 8)
    wrmsr
    mov eax, cr0
    or eax, (1 << 31)
    mov cr0, eax
    ; The CPU is now in compatibility mode.
    ; Still need to load the GDT with the 64-bit flags set in the code and data selectors.