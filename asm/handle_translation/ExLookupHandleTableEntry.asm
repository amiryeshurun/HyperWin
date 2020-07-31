nt!ExpLookupHandleTableEntry:
82c6d6d6 8bff            mov     edi,edi
82c6d6d8 55              push    ebp
82c6d6d9 8bec            mov     ebp,esp
82c6d6db 8b4508          mov     eax,dword ptr [ebp+8]  ; eax = Handle
82c6d6de 83e0fc          and     eax,0FFFFFFFCh  ; Clean last 2 bits, as all handles are a multiple of 4
82c6d6e1 3b4134          cmp     eax,dword ptr [ecx+34h] ; ecx = handle table
82c6d6e4 7204            jb      nt!ExpLookupHandleTableEntry+0x14 (82c6d6ea)

nt!ExpLookupHandleTableEntry+0x10: ; Failed, return 0 
82c6d6e6 33c0            xor     eax,eax
82c6d6e8 eb4b            jmp     nt!ExpLookupHandleTableEntry+0x5f (82c6d735)

nt!ExpLookupHandleTableEntry+0x14:
82c6d6ea 56              push    esi
82c6d6eb 8b31            mov     esi,dword ptr [ecx] ; esi = handle_table->TableCode
82c6d6ed 8bce            mov     ecx,esi ; ecx = table_code (tableBase)
82c6d6ef 83e103          and     ecx,3   ; ecx = tableLevel = table_code & 3
82c6d6f2 2bf1            sub     esi,ecx ; esi = (tableBase & ~3)
82c6d6f4 83e900          sub     ecx,0   ; if table_level == 0
82c6d6f7 7438            je      nt!ExpLookupHandleTableEntry+0x5b (82c6d731)

nt!ExpLookupHandleTableEntry+0x23:
82c6d6f9 49              dec     ecx
82c6d6fa 8bc8            mov     ecx,eax  ; Handle without 2 bits on the right
82c6d6fc 7420            je      nt!ExpLookupHandleTableEntry+0x48 (82c6d71e) ; if ecx (table level) was 1

nt!ExpLookupHandleTableEntry+0x28: ; table level was 2
82c6d6fe 81e1ff070000    and     ecx,7FFh ; save 12 right bits
82c6d704 2bc1            sub     eax,ecx  ; Handle without right 12 bits
82c6d706 c1e809          shr     eax,9    
82c6d709 8bd0            mov     edx,eax
82c6d70b 81e2ff0f0000    and     edx,0FFFh ; edx = eax & 0xfff
82c6d711 2bc2            sub     eax,edx   
82c6d713 c1e80a          shr     eax,0Ah
82c6d716 8b0406          mov     eax,dword ptr [esi+eax]
82c6d719 8b0402          mov     eax,dword ptr [edx+eax]
82c6d71c eb0e            jmp     nt!ExpLookupHandleTableEntry+0x56 (82c6d72c)

nt!ExpLookupHandleTableEntry+0x48: ; table level was 1
82c6d71e 81e1ff070000    and     ecx,7FFh
82c6d724 2bc1            sub     eax,ecx
82c6d726 c1e809          shr     eax,9
82c6d729 8b0406          mov     eax,dword ptr [esi+eax]

nt!ExpLookupHandleTableEntry+0x56:
82c6d72c 8d0448          lea     eax,[eax+ecx*2]
82c6d72f eb03            jmp     nt!ExpLookupHandleTableEntry+0x5e (82c6d734)

nt!ExpLookupHandleTableEntry+0x5b:
82c6d731 8d0446          lea     eax,[esi+eax*2] ; eax = *( tableBase + handle(cleaned 2 right bits) * 2)

nt!ExpLookupHandleTableEntry+0x5e:
82c6d734 5e              pop     esi

nt!ExpLookupHandleTableEntry+0x5f:
82c6d735 5d              pop     ebp
82c6d736 c20400          ret     4