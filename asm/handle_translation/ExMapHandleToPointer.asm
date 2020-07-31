82c6d5ff 8bff            mov     edi,edi
82c6d601 55              push    ebp
82c6d602 8bec            mov     ebp,esp
82c6d604 53              push    ebx
82c6d605 56              push    esi
82c6d606 33f6            xor     esi,esi
82c6d608 f74508fc070000  test    dword ptr [ebp+8],7FCh
82c6d60f 743a            je      nt!ExMapHandleToPointerEx+0x4c (82c6d64b)

nt!ExMapHandleToPointerEx+0x12:
82c6d611 ff7508          push    dword ptr [ebp+8]
82c6d614 8bcf            mov     ecx,edi    ; ecx is the handle table of the current process
82c6d616 e8bb000000      call    nt!ExpLookupHandleTableEntry (82c6d6d6)
82c6d61b 8bf0            mov     esi,eax    ; esi = result from handle table entry, a pointer
82c6d61d 85f6            test    esi,esi
82c6d61f 742a            je      nt!ExMapHandleToPointerEx+0x4c (82c6d64b)

nt!ExMapHandleToPointerEx+0x22:
82c6d621 8b0e            mov     ecx,dword ptr [esi] ; ecx contains "Object"
82c6d623 f6c101          test    cl,1                ; checks if the LSB is 1, if so, go to 0x29
82c6d626 7415            je      nt!ExMapHandleToPointerEx+0x3e (82c6d63d)

nt!ExMapHandleToPointerEx+0x29:
82c6d628 8d41ff          lea     eax,[ecx-1]         ; if the LSB is 1, then make it 0 (entry &= ~1)
82c6d62b 8bd0            mov     edx,eax             ; edx = object (without bit 1)
82c6d62d 8bde            mov     ebx,esi             ; ebx = Handle (hadnle table entry)
82c6d62f 8bc1            mov     eax,ecx             
82c6d631 f00fb113        lock cmpxchg dword ptr [ebx],edx
82c6d635 3bc1            cmp     eax,ecx
82c6d637 7508            jne     nt!ExMapHandleToPointerEx+0x42 (82c6d641)

nt!ExMapHandleToPointerEx+0x3a:
82c6d639 8bc6            mov     eax,esi             ; eax = handle table entry (ptr)
82c6d63b eb70            jmp     nt!ExMapHandleToPointerEx+0xae (82c6d6ad)

nt!ExMapHandleToPointerEx+0x3e:
82c6d63d 85c9            test    ecx,ecx  ; if ecx is 0, then go to 0x4c
82c6d63f 740a            je      nt!ExMapHandleToPointerEx+0x4c (82c6d64b)

nt!ExMapHandleToPointerEx+0x42:
82c6d641 56              push    esi
82c6d642 8bcf            mov     ecx,edi
82c6d644 e850690600      call    nt!ExpBlockOnLockedHandleEntry (82cd3f99) ; The handle is blocked, wait until it is released. We are passing the handle table as s parameter using ecx
82c6d649 ebd6            jmp     nt!ExMapHandleToPointerEx+0x22 (82c6d621) ; loop back

nt!ExMapHandleToPointerEx+0x4c:
82c6d64b 837f1c00        cmp     dword ptr [edi+1Ch],0
82c6d64f 745a            je      nt!ExMapHandleToPointerEx+0xac (82c6d6ab)

nt!ExMapHandleToPointerEx+0x52:
82c6d651 64a124010000    mov     eax,dword ptr fs:[00000124h]
82c6d657 6a03            push    3
82c6d659 ff7508          push    dword ptr [ebp+8]
82c6d65c 50              push    eax
82c6d65d 57              push    edi
82c6d65e e8b46e0e00      call    nt!ExpUpdateDebugInfo (82d54517)
82c6d663 807d0c01        cmp     byte ptr [ebp+0Ch],1
82c6d667 754a            jne     nt!ExMapHandleToPointerEx+0xb4 (82c6d6b3)

nt!ExMapHandleToPointerEx+0x6a:
82c6d669 64a124010000    mov     eax,dword ptr fs:[00000124h]
82c6d66f 80b83401000001  cmp     byte ptr [eax+134h],1
82c6d676 7433            je      nt!ExMapHandleToPointerEx+0xac (82c6d6ab)

nt!ExMapHandleToPointerEx+0x79:
82c6d678 f705a82abb8200010000 test dword ptr [nt!NtGlobalFlag (82bb2aa8)],100h
82c6d682 741d            je      nt!ExMapHandleToPointerEx+0xa2 (82c6d6a1)

nt!ExMapHandleToPointerEx+0x85:
82c6d684 64a124010000    mov     eax,dword ptr fs:[00000124h]
82c6d68a ff7050          push    dword ptr [eax+50h]
82c6d68d ff7508          push    dword ptr [ebp+8]
82c6d690 68f06ac682      push    offset nt! ?? ::NNGAKEGL::`string' (82c66af0)
82c6d695 6a00            push    0
82c6d697 6a5d            push    5Dh
82c6d699 e84e89e9ff      call    nt!DbgPrintEx (82b05fec)
82c6d69e 83c414          add     esp,14h

nt!ExMapHandleToPointerEx+0xa2:
82c6d6a1 68080000c0      push    0C0000008h
82c6d6a6 e85477ebff      call    nt!KeRaiseUserException (82b24dff)

nt!ExMapHandleToPointerEx+0xac:
82c6d6ab 33c0            xor     eax,eax

nt!ExMapHandleToPointerEx+0xae:
82c6d6ad 5e              pop     esi
82c6d6ae 5b              pop     ebx
82c6d6af 5d              pop     ebp
82c6d6b0 c20800          ret     8

nt!ExMapHandleToPointerEx+0xb4:
82c6d6b3 f705a82abb8200000040 test dword ptr [nt!NtGlobalFlag (82bb2aa8)],40000000h
82c6d6bd 74ec            je      nt!ExMapHandleToPointerEx+0xac (82c6d6ab)

nt!ExMapHandleToPointerEx+0xc0:
82c6d6bf 6a01            push    1
82c6d6c1 56              push    esi
82c6d6c2 57              push    edi
82c6d6c3 ff7508          push    dword ptr [ebp+8]
82c6d6c6 6893000000      push    93h
82c6d6cb e83234ebff      call    nt!KeBugCheckEx (82b20b02)
82c6d6d0 cc              int     3