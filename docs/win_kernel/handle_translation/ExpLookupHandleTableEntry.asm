nt!ExpLookupHandleTableEntry:
fffff803`334f01d0 8b01            mov     eax,dword ptr [rcx]
fffff803`334f01d2 4883e2fc        and     rdx,0FFFFFFFFFFFFFFFCh ; The last 2 bits (tag bits) must be 0
fffff803`334f01d6 483bd0          cmp     rdx,rax 
fffff803`334f01d9 735a            jae     nt!ExpLookupHandleTableEntry+0x65 (fffff803`334f0235)

nt!ExpLookupHandleTableEntry+0xb:
fffff803`334f01db 4c8b4108        mov     r8,qword ptr [rcx+8] ; get the table base
fffff803`334f01df 418bc0          mov     eax,r8d
fffff803`334f01e2 83e003          and     eax,3 ; get table level
fffff803`334f01e5 83f801          cmp     eax,1
fffff803`334f01e8 7518            jne     nt!ExpLookupHandleTableEntry+0x32 (fffff803`334f0202)

; r8 = table base
; eax = table level

nt!ExpLookupHandleTableEntry+0x1a:
fffff803`334f01ea 488bc2          mov     rax,rdx
fffff803`334f01ed 48c1e80a        shr     rax,0Ah
fffff803`334f01f1 81e2ff030000    and     edx,3FFh
fffff803`334f01f7 498b44c0ff      mov     rax,qword ptr [r8+rax*8-1]
fffff803`334f01fc 488d0490        lea     rax,[rax+rdx*4]
fffff803`334f0200 c3              ret

nt!ExpLookupHandleTableEntry+0x32:
fffff803`334f0202 85c0            test    eax,eax
fffff803`334f0204 7506            jne     nt!ExpLookupHandleTableEntry+0x3c (fffff803`334f020c)

nt!ExpLookupHandleTableEntry+0x36:
fffff803`334f0206 498d0490        lea     rax,[r8+rdx*4]
fffff803`334f020a c3              ret

nt!ExpLookupHandleTableEntry+0x3c:
fffff803`334f020c 488bca          mov     rcx,rdx
fffff803`334f020f 48c1e90a        shr     rcx,0Ah
fffff803`334f0213 488bc1          mov     rax,rcx
fffff803`334f0216 81e1ff010000    and     ecx,1FFh
fffff803`334f021c 48c1e809        shr     rax,9
fffff803`334f0220 81e2ff030000    and     edx,3FFh
fffff803`334f0226 498b44c0fe      mov     rax,qword ptr [r8+rax*8-2]
fffff803`334f022b 488b04c8        mov     rax,qword ptr [rax+rcx*8]
fffff803`334f022f 488d0490        lea     rax,[rax+rdx*4]
fffff803`334f0233 c3              ret

nt!ExpLookupHandleTableEntry+0x65:
fffff803`334f0235 33c0            xor     eax,eax
fffff803`334f0237 c3              ret