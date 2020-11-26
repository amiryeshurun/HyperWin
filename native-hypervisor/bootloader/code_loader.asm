%define EFER_MSR 0xC0000080
#define DISK_INDEX_ADDRESS 0x1550
%define SIZE_IN_SECTORS 0x1552
%define CURRENT_OFFSET 0x1556
%define BOOTABLE_SIGNATURE 0x55aa
%define CODE_BEGIN_ADDRESS 0x3300000
%define LARGE_PAGE_SIZE 0x200000
%define HYPERVISOR_SIZE 0xfffff
%define DAP_ADDRESS 0x1200
%define CODE_LOAD_ADDRESS 0xE000
%define BOOTLOADER_SIZE 0x1ff0
%define COM3 0x3E8
%define COMPUTER_MEM_SIZE 16

%macro MovQwordToAddressLittleEndian 3
    mov eax, %1
    mov dword [eax], %3
    mov dword [eax+4], %2
%endmacro

%macro ClearSelectorsUsingAx 0
    xor eax, eax
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax 
%endmacro

extern SectorsLoader

SEGMENT .text

; This code is suppose to run starting in address 0xE000 (CODE_LOAD_ADDRESS)
[BITS 16]
LoadHypervisorCode:
    ; Create a tempotarly GDT
    mov eax, 0x1000 ; gdt
    mov word [eax], 0x00ff ; limit
    MovQwordToAddressLittleEndian 0x1002, 0x0, 0x2000
    MovQwordToAddressLittleEndian 0x2000, 0x0, 0x0 ; null descriptor - 0
    MovQwordToAddressLittleEndian 0x2008, 0xa09b00, 0x0	; code - long mode - 8
    MovQwordToAddressLittleEndian 0x2010, 0xcf9300, 0xffff ; data - long and compatibility - 16
    MovQwordToAddressLittleEndian 0x2018, 0xcf9a00, 0xffff ; code - 32-bit mode - 24
    MovQwordToAddressLittleEndian 0x2020, 0x9a00, 0xffff ; code - 16 bit (not real) - 32
    MovQwordToAddressLittleEndian 0x2028, 0x9200, 0xffff ; data - 16 bit (not real) - 40

    cli
    lgdt [0x1000]
    ClearSelectorsUsingAx
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp 24:EnterProtectedMode

[BITS 32]
EnterProtectedMode:
    cli
    mov ax, 16
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov gs, ax
    mov fs, ax

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

    ; Enable Long Mode
    mov eax, cr0
    and eax, ~(1 << 31)
    mov cr0, eax
    mov eax, cr4
    or eax, (1 << 5)
    mov cr4, eax

    ; PML4
    mov eax, 0x17000
    mov cr3, eax

    ; EFER
    mov ecx, EFER_MSR
    rdmsr
    or eax, (1 << 8)
    wrmsr
    mov eax, cr0
    or eax, (1 << 31)
    mov cr0, eax
    jmp 8:AsmReturnFromRealModeFunction

BootloaderLongMode:
    cli
    mov ax, 16
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov rsp, 0x2800000
    call SectorsLoader
    
    