nt!KiFastCallEntry:
82a940c0 b923000000      mov     ecx,23h
82a940c5 6a30            push    30h
82a940c7 0fa1            pop     fs    ; load KPCR to fs
82a940c9 8ed9            mov     ds,cx
82a940cb 8ec1            mov     es,cx
82a940cd 648b0d40000000  mov     ecx,dword ptr fs:[40h] ; Load the task state segment
82a940d4 8b6104          mov     esp,dword ptr [ecx+4]  ; Load the Esp0 field to ESP
; The field SS0 contains the stack segment selector for CPL=0, and the field ESP0/RSP0 contains the new ESP/RSP value for CPL=0.
; When an interrupt happens in protected (32-bit) mode, the x86 CPU will look in the TSS for SS0 and ESP0 and load their values
; into SS and ESP respectively. This allows for the kernel to use a different stack than the user program,
; and also have this stack be unique for each user program.
82a940d7 6a23            push    23h
82a940d9 52              push    edx
82a940da 9c              pushfd         ; push the EFLAGS resigter to the stack
82a940db 6a02            push    2
82a940dd 83c208          add     edx,8  ; edx poinst to start of syscall params (ignore dual return address)
82a940e0 9d              popfd
82a940e1 804c240102      or      byte ptr [esp+1],2
82a940e6 6a1b            push    1Bh
82a940e8 ff350403dfff    push    dword ptr ds:[0FFDF0304h]
82a940ee 6a00            push    0
82a940f0 55              push    ebp
82a940f1 53              push    ebx
82a940f2 56              push    esi
82a940f3 57              push    edi
82a940f4 648b1d1c000000  mov     ebx,dword ptr fs:[1Ch]     ; ebx = KPCR
82a940fb 6a3b            push    3Bh
82a940fd 8bb324010000    mov     esi,dword ptr [ebx+124h]   ; esi = KTHREAD of the current thread
82a94103 ff33            push    dword ptr [ebx]            
82a94105 c703ffffffff    mov     dword ptr [ebx],0FFFFFFFFh
82a9410b 8b6e28          mov     ebp,dword ptr [esi+28h]    ; ebp = pointer to the initial stack
82a9410e 6a01            push    1
82a94110 83ec48          sub     esp,48h
82a94113 81ed9c020000    sub     ebp,29Ch
82a94119 c6863a01000001  mov     byte ptr [esi+13Ah],1      ; KPROCESSOR_MODE = 1, indicates that the previous thread called the current
                                                            ; kernel thread from usermode
82a94120 3bec            cmp     ebp,esp                                ; no clue
82a94122 7597            jne     nt!KiFastCallEntry2+0x49 (82a940bb)    ; ^^^^^^^
82a94124 83652c00        and     dword ptr [ebp+2Ch],0
82a94128 f64603df        test    byte ptr [esi+3],0DFh                  ; not sure
82a9412c 89ae28010000    mov     dword ptr [esi+128h],ebp               
82a94132 0f8538feffff    jne     nt!Dr_FastCallDrSave (82a93f70)
82a94138 8b5d60          mov     ebx,dword ptr [ebp+60h]
82a9413b 8b7d68          mov     edi,dword ptr [ebp+68h]
82a9413e 89550c          mov     dword ptr [ebp+0Ch],edx
82a94141 c74508000ddbba  mov     dword ptr [ebp+8],0BADB0D00h
82a94148 895d00          mov     dword ptr [ebp],ebx
82a9414b 897d04          mov     dword ptr [ebp+4],edi
82a9414e fb              sti
82a9414f 8bf8            mov     edi,eax    ; syscall id
82a94151 c1ef08          shr     edi,8     
82a94154 83e710          and     edi,10h    ; edi now contains either 10h or 0h
82a94157 8bcf            mov     ecx,edi    ; move SSDT number to ecx
82a94159 03bebc000000    add     edi,dword ptr [esi+0BCh] ; edi = (type -> 10h / 0h) + SSDT
82a9415f 8bd8            mov     ebx,eax
82a94161 25ff0f0000      and     eax,0FFFh
82a94166 3b4708          cmp     eax,dword ptr [edi+8]
82a94169 0f8333fdffff    jae     nt!KiBBTUnexpectedRange (82a93ea2) ; unallowed syscall id
82a9416f 83f910          cmp     ecx,10h                            ; second SSDT
82a94172 751a            jne     nt!KiFastCallEntry+0xce (82a9418e) ; first SSDT
82a94174 8b8e88000000    mov     ecx,dword ptr [esi+88h]            
82a9417a 33f6            xor     esi,esi
82a9417c 0bb1700f0000    or      esi,dword ptr [ecx+0F70h]
82a94182 740a            je      nt!KiFastCallEntry+0xce (82a9418e)
82a94184 52              push    edx
82a94185 50              push    eax
82a94186 ff154c09bc82    call    dword ptr [nt!KeGdiFlushUserBatch (82bc094c)]
82a9418c 58              pop     eax
82a9418d 5a              pop     edx
82a9418e 64ff05b0060000  inc     dword ptr fs:[6B0h]      ; FIRST SSDT ENTRYPOINT: system call counter
82a94195 8bf2            mov     esi,edx                  ; esi = beginning of params in user mode
82a94197 33c9            xor     ecx,ecx
82a94199 8b570c          mov     edx,dword ptr [edi+0Ch]  ; edx = Arguments Table
82a9419c 8b3f            mov     edi,dword ptr [edi]      ; edi = ServiceTable Array
82a9419e 8a0c10          mov     cl,byte ptr [eax+edx]    ; It is an array of single byte values, cx now contains the number
                                                          ; of arguments to the syscall
82a941a1 8b1487          mov     edx,dword ptr [edi+eax*4] ; convert the syscall id to an index in the table, edx now points
                                                           ; to the handler of the systemcall
82a941a4 2be1            sub     esp,ecx
82a941a6 c1e902          shr     ecx,2
82a941a9 8bfc            mov     edi,esp
82a941ab 3b351c07bc82    cmp     esi,dword ptr [nt!MmUserProbeAddress (82bc071c)] ; some wrong user address
82a941b1 0f832e020000    jae     nt!KiSystemCallExit2+0xa5 (82a943e5)             ; exit
82a941b7 f3a5            rep movs dword ptr es:[edi],dword ptr [esi]              ; copy args to kernel stuck
82a941b9 f6456c01        test    byte ptr [ebp+6Ch],1
82a941bd 7416            je      nt!KiFastCallEntry+0x115 (82a941d5)
82a941bf 648b0d24010000  mov     ecx,dword ptr fs:[124h]
82a941c6 8b3c24          mov     edi,dword ptr [esp]
82a941c9 89993c010000    mov     dword ptr [ecx+13Ch],ebx
82a941cf 89b92c010000    mov     dword ptr [ecx+12Ch],edi
82a941d5 8bda            mov     ebx,edx
82a941d7 f60508d9b88240  test    byte ptr [nt!PerfGlobalGroupMask+0x8 (82b8d908)],40h
82a941de 0f954512        setne   byte ptr [ebp+12h]
82a941e2 0f858c030000    jne     nt!KiServiceExit2+0x17b (82a94574)
82a941e8 ffd3            call    ebx                                              ; call the syscall handler. Args should be available using esi
82a941ea f6456c01        test    byte ptr [ebp+6Ch],1
82a941ee 7434            je      nt!KiFastCallEntry+0x164 (82a94224)
82a941f0 8bf0            mov     esi,eax
82a941f2 ff156871a582    call    dword ptr [nt!_imp__KeGetCurrentIrql (82a57168)]
82a941f8 0ac0            or      al,al
82a941fa 0f853b030000    jne     nt!KiServiceExit2+0x142 (82a9453b)
82a94200 8bc6            mov     eax,esi
82a94202 648b0d24010000  mov     ecx,dword ptr fs:[124h]
82a94209 f68134010000ff  test    byte ptr [ecx+134h],0FFh
82a94210 0f8543030000    jne     nt!KiServiceExit2+0x160 (82a94559)
82a94216 8b9184000000    mov     edx,dword ptr [ecx+84h]
82a9421c 0bd2            or      edx,edx
82a9421e 0f8535030000    jne     nt!KiServiceExit2+0x160 (82a94559)
82a94224 8be5            mov     esp,ebp
82a94226 807d1200        cmp     byte ptr [ebp+12h],0
82a9422a 0f8550030000    jne     nt!KiServiceExit2+0x187 (82a94580)
82a94230 648b0d24010000  mov     ecx,dword ptr fs:[124h]
82a94237 8b553c          mov     edx,dword ptr [ebp+3Ch]
82a9423a 899128010000    mov     dword ptr [ecx+128h],edx


* To inject a syscall, we can get the SSDT using the KTHREAD:
(KTHREAD+0BCh) = address of KeServiceDescriptorTable
[(KTHREAD+0BCh)] = array of hadlers
The above[<SYSCALL_ID> * 4] = handler function, can be set to an invalid address to cause a VM exit