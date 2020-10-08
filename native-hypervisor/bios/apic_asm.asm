%define COM1 0x3F8
%define COM2 0x2F8
%define COM3 0x3E8
%define COM4 0x2E8
%define EFER_MSR 0xC0000080
%define CPU_DATA_ADDRESS 0x4000
%define APIC_FUNC_ADDRESS 0x6000

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

SECTION .text

global ApicStart
global ApicEnd

extern InitializeSingleHypervisor

[BITS 16]
ApicStart:
    ; Enter protected mode
    cli
    lgdt [0x1000]
    mov ax, 0
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp 24:(ApicEnableLongMode - ApicStart + APIC_FUNC_ADDRESS)

[BITS 32]
ApicEnableLongMode:
    ; Enable paging, PAE and enter long mode
    mov ax, 16
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov gs, ax
    mov fs, ax
    mov eax, cr0
    and eax, ~(1 << 31)
    mov cr0, eax
    mov eax, cr4
    or eax, (1 << 5)
    mov cr4, eax
    SetCr3BasePhysicalAddress 0x17000
    mov ecx, EFER_MSR
    rdmsr
    or eax, (1 << 8)
    wrmsr
    mov eax, cr0
    or eax, (1 << 31)
    mov cr0, eax
    jmp 8:(ApicLongMode - ApicStart + APIC_FUNC_ADDRESS)

[BITS 64]
ApicLongMode:
    mov rsp, 0x770000
    mov rdi, qword [CPU_DATA_ADDRESS]
    call InitializeSingleHypervisor
    xor rax, rax
    xor rbx, rbx
    xor rcx, rcx
    xor rdx, rdx
    xor rdi, rdi
    xor rsi, rsi
    xor rsp, rsp
    xor rbp, rbp
    xor r8, r8
    xor r9, r9
    xor r10, r10
    xor r11, r11
    xor r12, r12
    xor r13, r13
    xor r14, r14
    xor r15, r15
    ; bye bye...
    hlt
ApicEnd: