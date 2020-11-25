%define EFER_MSR 0xC0000080
%define MBR_ADDRESS 0x7c00
%define DAP_ADDRESS 0x4000
%define FIRST_SECTOR_DEST 0x3000
%define BOOTABLE_SIGNATURE 0x55aa
%define REAL_MODE_OUTPUT_BUFFER_ADDRESS 0x2200
%define REAL_MODE_CODE_START 0x4200
%define WINDOWS_DISK_INDEX 0x6010
%define CODE_BEGIN_ADDRESS 0x3300000
%define COMPUTER_MEM_SIZE 16
%define LARGE_PAGE_SIZE 0x200000
%define IVT_ADDRESS 0x7500
%define ISO_IMAGE_SIZE 0xfffff
%define COM1 0x3F8
%define COM2 0x2F8
%define COM3 0x3E8
%define COM4 0x2E8

; Bootloader entrypoint
[BITS 16]
BiosFirstEntry:
    mov word [REAL_MODE_OUTPUT_BUFFER_ADDRESS], dl ; save the disk number
    xor edx, edx
    mov eax, ISO_IMAGE_SIZE
    mov ecx, 0x200 ; sector size
    div ecx
    inc eax ; eax now stores the number of sectors to read


