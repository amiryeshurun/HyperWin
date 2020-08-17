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
