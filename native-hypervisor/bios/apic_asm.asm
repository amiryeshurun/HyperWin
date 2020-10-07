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

global ApicStart
global ApicEnd

ApicStart:
    OutputSerial 'H'
    hlt
ApicEnd: