%define EFER_MSR 0xC0000080
#define DISK_INDEX_ADDRESS 0x1550
%define SIZE_IN_SECTORS 0x1552
%define CURRENT_OFFSET 0x1556
%define BOOTABLE_SIGNATURE 0xaa55
%define CODE_BEGIN_ADDRESS 0x3300000
%define LARGE_PAGE_SIZE 0x200000
%define HYPERVISOR_SIZE 0xfffff
%define DAP_ADDRESS 0x1200
%define CODE_LOAD_ADDRESS 0xE000
%define BOOTLOADER_SIZE 0x1ff0
%define COM3 0x3E8
%define COMPUTER_MEM_SIZE 16
%define SECTOR_SIZE 512

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

SEGMENT .text

; Bootloader entrypoint
[BITS 16]
BiosFirstEntry:
    mov byte [DISK_INDEX_ADDRESS], dl ; save the disk number
    mov eax, BOOTLOADER_SIZE
    mov ecx, SECTOR_SIZE ; sector size
    div ecx ; eax now stores the number of sectors to read for bootloader
    mov ecx, eax ; for the below loop

    mov eax, CODE_LOAD_ADDRESS
    mov ebx, 1
.ReadBootloaderCode:
    ; Prepare DAP
    mov byte [DAP_ADDRESS], 0x10  ; packet size. always 0x10
    mov byte [DAP_ADDRESS + 1], 0 ; unused. always 0
    mov word [DAP_ADDRESS + 2], 1 ; number of sectors to be read
    mov word [DAP_ADDRESS + 4], eax ; dest address
    mov word [DAP_ADDRESS + 6], 0 ; segment dest
    mov word [DAP_ADDRESS + 4], ebx ; sector no. to be read
    add eax, 0x200 ; sector size
    mov dword [CURRENT_OFFSET], eax
    mov ah, 0x42 ; Extended read sectors BIOS magic value
    int 0x13 ; Extended read sectors BIOS interrupt
    mov eax, dword [CURRENT_OFFSET]
    inc ebx ; next sector
    loop .ReadBootloaderCode

    ; Jump to the main bootloader code
    jmp 0:CODE_LOAD_ADDRESS ; next sectors are stored here

times 510 - ($-$$) db 0 ; Zero the remaining area
dw BOOTABLE_SIGNATURE