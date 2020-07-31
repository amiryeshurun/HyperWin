nt!ObReferenceObjectByHandleWithTag:
82c6d325 8bff            mov     edi,edi
82c6d327 55              push    ebp
82c6d328 8bec            mov     ebp,esp
82c6d32a 83e4f8          and     esp,0FFFFFFF8h
82c6d32d 83ec1c          sub     esp,1Ch
82c6d330 8b5508          mov     edx,dword ptr [ebp+8]   ; edx = IN Handle
82c6d333 53              push    ebx
82c6d334 8b5d1c          mov     ebx,dword ptr [ebp+1Ch] ; ebx = OUT POBJECT_HANDLE_INFORMATION HandleInformation  
82c6d337 56              push    esi
82c6d338 648b3524010000  mov     esi,dword ptr fs:[124h] 
82c6d33f 57              push    edi  
82c6d340 8b7e50          mov     edi,dword ptr [esi+50h]
82c6d343 832300          and     dword ptr [ebx],0
82c6d346 897c2424        mov     dword ptr [esp+24h],edi
82c6d34a c644241300      mov     byte ptr [esp+13h],0
82c6d34f 85d2            test    edx,edx
82c6d351 0f8df7000000    jge     nt!ObReferenceObjectByHandleWithTag+0x127 (82c6d44e) ; if handle is positive

nt!ObReferenceObjectByHandleWithTag+0x32:
82c6d357 83faff          cmp     edx,0FFFFFFFFh           ; HANDLE = -1
82c6d35a 7568            jne     nt!ObReferenceObjectByHandleWithTag+0x9f (82c6d3c4)

nt!ObReferenceObjectByHandleWithTag+0x37:
82c6d35c 8b4510          mov     eax,dword ptr [ebp+10h]
82c6d35f 3b050431bb82    cmp     eax,dword ptr [nt!PsProcessType (82bb3104)]
82c6d365 740e            je      nt!ObReferenceObjectByHandleWithTag+0x50 (82c6d375)

nt!ObReferenceObjectByHandleWithTag+0x42:
82c6d367 85c0            test    eax,eax
82c6d369 740a            je      nt!ObReferenceObjectByHandleWithTag+0x50 (82c6d375)

nt!ObReferenceObjectByHandleWithTag+0x46:
82c6d36b b8240000c0      mov     eax,0C0000024h
82c6d370 e97c020000      jmp     nt!ObReferenceObjectByHandleWithTag+0x2ca (82c6d5f1)

nt!ObReferenceObjectByHandleWithTag+0x50:
82c6d375 8b7650          mov     esi,dword ptr [esi+50h]
82c6d378 eb5e            jmp     nt!ObReferenceObjectByHandleWithTag+0xb3 (82c6d3d8)

nt!ObReferenceObjectByHandleWithTag+0x55:
82c6d37a b8220000c0      mov     eax,0C0000022h
82c6d37f e96d020000      jmp     nt!ObReferenceObjectByHandleWithTag+0x2ca (82c6d5f1)

nt!ObReferenceObjectByHandleWithTag+0x5f:
82c6d384 8b4520          mov     eax,dword ptr [ebp+20h]
82c6d387 8d7ee8          lea     edi,[esi-18h]
82c6d38a 85c0            test    eax,eax
82c6d38c 740a            je      nt!ObReferenceObjectByHandleWithTag+0x73 (82c6d398)

nt!ObReferenceObjectByHandleWithTag+0x69:
82c6d38e 832000          and     dword ptr [eax],0
82c6d391 c74004ffff1f00  mov     dword ptr [eax+4],1FFFFFh

nt!ObReferenceObjectByHandleWithTag+0x73:
82c6d398 833d801cb88200  cmp     dword ptr [nt!ObpTraceFlags (82b81c80)],0
82c6d39f 7413            je      nt!ObReferenceObjectByHandleWithTag+0x8f (82c6d3b4)

nt!ObReferenceObjectByHandleWithTag+0x7c:
82c6d3a1 f6470d01        test    byte ptr [edi+0Dh],1
82c6d3a5 740d            je      nt!ObReferenceObjectByHandleWithTag+0x8f (82c6d3b4)

nt!ObReferenceObjectByHandleWithTag+0x82:
82c6d3a7 ff7518          push    dword ptr [ebp+18h]
82c6d3aa 6a01            push    1
82c6d3ac 6a01            push    1
82c6d3ae 57              push    edi
82c6d3af e88aa0ecff      call    nt!ObpPushStackInfo (82b3743e)

nt!ObReferenceObjectByHandleWithTag+0x8f:
82c6d3b4 33c0            xor     eax,eax
82c6d3b6 40              inc     eax
82c6d3b7 f00fc107        lock xadd dword ptr [edi],eax
82c6d3bb 8933            mov     dword ptr [ebx],esi
82c6d3bd 33c0            xor     eax,eax
82c6d3bf e92d020000      jmp     nt!ObReferenceObjectByHandleWithTag+0x2ca (82c6d5f1)

nt!ObReferenceObjectByHandleWithTag+0x9f:
82c6d3c4 83fafe          cmp     edx,0FFFFFFFEh             ; HANDLE = -2
82c6d3c7 7522            jne     nt!ObReferenceObjectByHandleWithTag+0xc4 (82c6d3eb)

nt!ObReferenceObjectByHandleWithTag+0xa4:
82c6d3c9 8b4510          mov     eax,dword ptr [ebp+10h]
82c6d3cc 3b050031bb82    cmp     eax,dword ptr [nt!PsThreadType (82bb3100)]
82c6d3d2 7404            je      nt!ObReferenceObjectByHandleWithTag+0xb3 (82c6d3d8)

nt!ObReferenceObjectByHandleWithTag+0xaf:
82c6d3d4 85c0            test    eax,eax
82c6d3d6 7593            jne     nt!ObReferenceObjectByHandleWithTag+0x46 (82c6d36b)

nt!ObReferenceObjectByHandleWithTag+0xb3:
82c6d3d8 f7450c0000e0ff  test    dword ptr [ebp+0Ch],0FFE00000h
82c6d3df 74a3            je      nt!ObReferenceObjectByHandleWithTag+0x5f (82c6d384)

nt!ObReferenceObjectByHandleWithTag+0xbc:
82c6d3e1 807d1400        cmp     byte ptr [ebp+14h],0
82c6d3e5 749d            je      nt!ObReferenceObjectByHandleWithTag+0x5f (82c6d384)

nt!ObReferenceObjectByHandleWithTag+0xc2:
82c6d3e7 eb91            jmp     nt!ObReferenceObjectByHandleWithTag+0x55 (82c6d37a)

nt!ObReferenceObjectByHandleWithTag+0xc4:
82c6d3eb 807d1400        cmp     byte ptr [ebp+14h],0      ; else if (AccessMode == KernelMode)
82c6d3ef 0f85ab000000    jne     nt!ObReferenceObjectByHandleWithTag+0x179 (82c6d4a0)

nt!ObReferenceObjectByHandleWithTag+0xce:
82c6d3f5 a120c9b882      mov     eax,dword ptr [nt!ObpKernelHandleTable (82b8c920)]
82c6d3fa 81f200000080    xor     edx,80000000h
82c6d400 895508          mov     dword ptr [ebp+8],edx   ; override the original IN Handle
82c6d403 89442414        mov     dword ptr [esp+14h],eax ; esp + 14 = Handle Table (in case of kernel access)

nt!ObReferenceObjectByHandleWithTag+0xe0:
82c6d407 66ff8e84000000  dec     word ptr [esi+84h]
82c6d40e ff7514          push    dword ptr [ebp+14h]     ; pass AccessMode to ExMapHandlerToPointerEx
82c6d411 8b7c2418        mov     edi,dword ptr [esp+18h] ; Handle Table
82c6d415 ff7508          push    dword ptr [ebp+8]       ; HANDLE
82c6d418 e8e2010000      call    nt!ExMapHandleToPointerEx (82c6d5ff)
82c6d41d 8bf8            mov     edi,eax ; edi is the entry
82c6d41f 85ff            test    edi,edi ; if entry doesnt exist (null)
82c6d421 0f848a010000    je      nt!ObReferenceObjectByHandleWithTag+0x28a (82c6d5b1)

nt!ObReferenceObjectByHandleWithTag+0x100:
82c6d427 8b0f            mov     ecx,dword ptr [edi] ; access the body ptr
82c6d429 8b4510          mov     eax,dword ptr [ebp+10h] 
82c6d42c 83e1f8          and     ecx,0FFFFFFF8h
82c6d42f 0fb6510c        movzx   edx,byte ptr [ecx+0Ch]
82c6d433 894c2418        mov     dword ptr [esp+18h],ecx
82c6d437 39049560d7b882  cmp     dword ptr nt!ObTypeIndexTable (82b8d760)[edx*4],eax
82c6d43e 746a            je      nt!ObReferenceObjectByHandleWithTag+0x183 (82c6d4aa)

nt!ObReferenceObjectByHandleWithTag+0x119:
82c6d440 85c0            test    eax,eax
82c6d442 7466            je      nt!ObReferenceObjectByHandleWithTag+0x183 (82c6d4aa)

nt!ObReferenceObjectByHandleWithTag+0x11d:
82c6d444 bb240000c0      mov     ebx,0C0000024h
82c6d449 e944010000      jmp     nt!ObReferenceObjectByHandleWithTag+0x26b (82c6d592)

nt!ObReferenceObjectByHandleWithTag+0x127: ; ELSE - handle is positive
82c6d44e 833de8acb88200  cmp     dword ptr [nt!ViVerifierDriverAddedThunkListHead (82b8ace8)],0
82c6d455 740b            je      nt!ObReferenceObjectByHandleWithTag+0x13b (82c6d462)

nt!ObReferenceObjectByHandleWithTag+0x130:
82c6d457 807d1400        cmp     byte ptr [ebp+14h],0
82c6d45b 7505            jne     nt!ObReferenceObjectByHandleWithTag+0x13b (82c6d462)

nt!ObReferenceObjectByHandleWithTag+0x136:
82c6d45d e8ebf11100      call    nt!VfCheckUserHandle (82d8c64d)

nt!ObReferenceObjectByHandleWithTag+0x13b:
82c6d462 3bbe50010000    cmp     edi,dword ptr [esi+150h]
82c6d468 740e            je      nt!ObReferenceObjectByHandleWithTag+0x151 (82c6d478)

nt!ObReferenceObjectByHandleWithTag+0x143:
82c6d46a 8bc7            mov     eax,edi
82c6d46c e84c240100      call    nt!ObReferenceProcessHandleTable (82c7f8bd)
82c6d471 c644241301      mov     byte ptr [esp+13h],1
82c6d476 eb06            jmp     nt!ObReferenceObjectByHandleWithTag+0x157 (82c6d47e)

nt!ObReferenceObjectByHandleWithTag+0x151:
82c6d478 8b87f4000000    mov     eax,dword ptr [edi+0F4h]

nt!ObReferenceObjectByHandleWithTag+0x157:
82c6d47e 89442414        mov     dword ptr [esp+14h],eax
82c6d482 85c0            test    eax,eax
82c6d484 741a            je      nt!ObReferenceObjectByHandleWithTag+0x179 (82c6d4a0)

nt!ObReferenceObjectByHandleWithTag+0x15f:
82c6d486 3b0520c9b882    cmp     eax,dword ptr [nt!ObpKernelHandleTable (82b8c920)]
82c6d48c 0f8575ffffff    jne     nt!ObReferenceObjectByHandleWithTag+0xe0 (82c6d407)

nt!ObReferenceObjectByHandleWithTag+0x16b:
82c6d492 807c241301      cmp     byte ptr [esp+13h],1
82c6d497 7507            jne     nt!ObReferenceObjectByHandleWithTag+0x179 (82c6d4a0)

nt!ObReferenceObjectByHandleWithTag+0x172:
82c6d499 8bcf            mov     ecx,edi
82c6d49b e885240100      call    nt!ObDereferenceProcessHandleTable (82c7f925)

nt!ObReferenceObjectByHandleWithTag+0x179:
82c6d4a0 b8080000c0      mov     eax,0C0000008h     ; return STATUS_INVALID_HANDLE
82c6d4a5 e947010000      jmp     nt!ObReferenceObjectByHandleWithTag+0x2ca (82c6d5f1)

nt!ObReferenceObjectByHandleWithTag+0x183:
82c6d4aa f705a82abb8200200000 test dword ptr [nt!NtGlobalFlag (82bb2aa8)],2000h
82c6d4b4 7412            je      nt!ObReferenceObjectByHandleWithTag+0x1a1 (82c6d4c8)

nt!ObReferenceObjectByHandleWithTag+0x18f:
82c6d4b6 0fb74704        movzx   eax,word ptr [edi+4]
82c6d4ba 50              push    eax
82c6d4bb e823530b00      call    nt!ObpTranslateGrantedAccessIndex (82d227e3)
82c6d4c0 8b4c2418        mov     ecx,dword ptr [esp+18h]
82c6d4c4 8bd8            mov     ebx,eax
82c6d4c6 eb0b            jmp     nt!ObReferenceObjectByHandleWithTag+0x1ac (82c6d4d3)

nt!ObReferenceObjectByHandleWithTag+0x1a1:
82c6d4c8 8b1d2864b782    mov     ebx,dword ptr [nt!ObpAccessProtectCloseBit (82b76428)]
82c6d4ce f7d3            not     ebx
82c6d4d0 235f04          and     ebx,dword ptr [edi+4]

nt!ObReferenceObjectByHandleWithTag+0x1ac:
82c6d4d3 8bc3            mov     eax,ebx
82c6d4d5 f7d0            not     eax
82c6d4d7 85450c          test    dword ptr [ebp+0Ch],eax
82c6d4da 7410            je      nt!ObReferenceObjectByHandleWithTag+0x1c5 (82c6d4ec)

nt!ObReferenceObjectByHandleWithTag+0x1b5:
82c6d4dc 807d1400        cmp     byte ptr [ebp+14h],0
82c6d4e0 740a            je      nt!ObReferenceObjectByHandleWithTag+0x1c5 (82c6d4ec)

nt!ObReferenceObjectByHandleWithTag+0x1bb:
82c6d4e2 bb220000c0      mov     ebx,0C0000022h
82c6d4e7 e9a6000000      jmp     nt!ObReferenceObjectByHandleWithTag+0x26b (82c6d592)

nt!ObReferenceObjectByHandleWithTag+0x1c5:
82c6d4ec 8b542414        mov     edx,dword ptr [esp+14h]
82c6d4f0 837a2000        cmp     dword ptr [edx+20h],0
82c6d4f4 7414            je      nt!ObReferenceObjectByHandleWithTag+0x1e3 (82c6d50a)

nt!ObReferenceObjectByHandleWithTag+0x1cf:
82c6d4f6 8b4508          mov     eax,dword ptr [ebp+8]
82c6d4f9 8bca            mov     ecx,edx
82c6d4fb e8207a0000      call    nt!ExpGetHandleInfo (82c74f20)
82c6d500 8b4c2418        mov     ecx,dword ptr [esp+18h]
82c6d504 8944241c        mov     dword ptr [esp+1Ch],eax
82c6d508 eb05            jmp     nt!ObReferenceObjectByHandleWithTag+0x1e8 (82c6d50f)

nt!ObReferenceObjectByHandleWithTag+0x1e3:
82c6d50a 8364241c00      and     dword ptr [esp+1Ch],0

nt!ObReferenceObjectByHandleWithTag+0x1e8:
82c6d50f 8b5520          mov     edx,dword ptr [ebp+20h]
82c6d512 85d2            test    edx,edx
82c6d514 741c            je      nt!ObReferenceObjectByHandleWithTag+0x20b (82c6d532)

nt!ObReferenceObjectByHandleWithTag+0x1ef:
82c6d516 a12864b782      mov     eax,dword ptr [nt!ObpAccessProtectCloseBit (82b76428)]
82c6d51b 895a04          mov     dword ptr [edx+4],ebx
82c6d51e 854704          test    dword ptr [edi+4],eax
82c6d521 8b07            mov     eax,dword ptr [edi]
82c6d523 7408            je      nt!ObReferenceObjectByHandleWithTag+0x206 (82c6d52d)

nt!ObReferenceObjectByHandleWithTag+0x1fe:
82c6d525 83e006          and     eax,6
82c6d528 83c801          or      eax,1
82c6d52b eb03            jmp     nt!ObReferenceObjectByHandleWithTag+0x209 (82c6d530)

nt!ObReferenceObjectByHandleWithTag+0x206:
82c6d52d 83e006          and     eax,6

nt!ObReferenceObjectByHandleWithTag+0x209:
82c6d530 8902            mov     dword ptr [edx],eax

nt!ObReferenceObjectByHandleWithTag+0x20b:
82c6d532 f60704          test    byte ptr [edi],4
82c6d535 7428            je      nt!ObReferenceObjectByHandleWithTag+0x238 (82c6d55f)

nt!ObReferenceObjectByHandleWithTag+0x210:
82c6d537 8b44241c        mov     eax,dword ptr [esp+1Ch]
82c6d53b 85c0            test    eax,eax
82c6d53d 7420            je      nt!ObReferenceObjectByHandleWithTag+0x238 (82c6d55f)

nt!ObReferenceObjectByHandleWithTag+0x218:
82c6d53f 833800          cmp     dword ptr [eax],0
82c6d542 741b            je      nt!ObReferenceObjectByHandleWithTag+0x238 (82c6d55f)

nt!ObReferenceObjectByHandleWithTag+0x21d:
82c6d544 837d0c00        cmp     dword ptr [ebp+0Ch],0
82c6d548 7415            je      nt!ObReferenceObjectByHandleWithTag+0x238 (82c6d55f)

nt!ObReferenceObjectByHandleWithTag+0x223:
82c6d54a ff750c          push    dword ptr [ebp+0Ch]
82c6d54d 8b542420        mov     edx,dword ptr [esp+20h]
82c6d551 ff7508          push    dword ptr [ebp+8]
82c6d554 8bc1            mov     eax,ecx
82c6d556 e83187fcff      call    nt!ObpAuditObjectAccess (82c35c8c)
82c6d55b 8b4c2418        mov     ecx,dword ptr [esp+18h]

nt!ObReferenceObjectByHandleWithTag+0x238:
82c6d55f 833d801cb88200  cmp     dword ptr [nt!ObpTraceFlags (82b81c80)],0
82c6d566 7417            je      nt!ObReferenceObjectByHandleWithTag+0x258 (82c6d57f)

nt!ObReferenceObjectByHandleWithTag+0x241:
82c6d568 f6410d01        test    byte ptr [ecx+0Dh],1
82c6d56c 7411            je      nt!ObReferenceObjectByHandleWithTag+0x258 (82c6d57f)

nt!ObReferenceObjectByHandleWithTag+0x247:
82c6d56e ff7518          push    dword ptr [ebp+18h]
82c6d571 6a01            push    1
82c6d573 6a01            push    1
82c6d575 51              push    ecx
82c6d576 e8c39eecff      call    nt!ObpPushStackInfo (82b3743e)
82c6d57b 8b4c2418        mov     ecx,dword ptr [esp+18h]

nt!ObReferenceObjectByHandleWithTag+0x258:
82c6d57f 33d2            xor     edx,edx
82c6d581 8bc1            mov     eax,ecx
82c6d583 42              inc     edx
82c6d584 f00fc110        lock xadd dword ptr [eax],edx
82c6d588 8b451c          mov     eax,dword ptr [ebp+1Ch]
82c6d58b 83c118          add     ecx,18h
82c6d58e 8908            mov     dword ptr [eax],ecx
82c6d590 33db            xor     ebx,ebx

nt!ObReferenceObjectByHandleWithTag+0x26b:
82c6d592 33c0            xor     eax,eax
82c6d594 40              inc     eax
82c6d595 f00907          lock or dword ptr [edi],eax
82c6d598 8b4c2414        mov     ecx,dword ptr [esp+14h]
82c6d59c 83c118          add     ecx,18h
82c6d59f 87442420        xchg    eax,dword ptr [esp+20h]
82c6d5a3 833900          cmp     dword ptr [ecx],0
82c6d5a6 740e            je      nt!ObReferenceObjectByHandleWithTag+0x28f (82c6d5b6)

nt!ObReferenceObjectByHandleWithTag+0x281:
82c6d5a8 33d2            xor     edx,edx
82c6d5aa e844bde6ff      call    nt!ExfUnblockPushLock (82ad92f3)
82c6d5af eb05            jmp     nt!ObReferenceObjectByHandleWithTag+0x28f (82c6d5b6)

nt!ObReferenceObjectByHandleWithTag+0x28a:
82c6d5b1 bb080000c0      mov     ebx,0C0000008h

nt!ObReferenceObjectByHandleWithTag+0x28f:
82c6d5b6 66ff8684000000  inc     word ptr [esi+84h]
82c6d5bd 0fb78684000000  movzx   eax,word ptr [esi+84h]
82c6d5c4 6685c0          test    ax,ax
82c6d5c7 7516            jne     nt!ObReferenceObjectByHandleWithTag+0x2b8 (82c6d5df)

nt!ObReferenceObjectByHandleWithTag+0x2a2:
82c6d5c9 8d4640          lea     eax,[esi+40h]
82c6d5cc 3900            cmp     dword ptr [eax],eax
82c6d5ce 740f            je      nt!ObReferenceObjectByHandleWithTag+0x2b8 (82c6d5df)

nt!ObReferenceObjectByHandleWithTag+0x2a9:
82c6d5d0 6683be8600000000 cmp     word ptr [esi+86h],0
82c6d5d8 7505            jne     nt!ObReferenceObjectByHandleWithTag+0x2b8 (82c6d5df)

nt!ObReferenceObjectByHandleWithTag+0x2b3:
82c6d5da e87bafdfff      call    nt!KiCheckForKernelApcDelivery (82a6855a)

nt!ObReferenceObjectByHandleWithTag+0x2b8:
82c6d5df 807c241301      cmp     byte ptr [esp+13h],1
82c6d5e4 7509            jne     nt!ObReferenceObjectByHandleWithTag+0x2c8 (82c6d5ef)

nt!ObReferenceObjectByHandleWithTag+0x2bf:
82c6d5e6 8b4c2424        mov     ecx,dword ptr [esp+24h]
82c6d5ea e836230100      call    nt!ObDereferenceProcessHandleTable (82c7f925)

nt!ObReferenceObjectByHandleWithTag+0x2c8:
82c6d5ef 8bc3            mov     eax,ebx

nt!ObReferenceObjectByHandleWithTag+0x2ca:
82c6d5f1 5f              pop     edi
82c6d5f2 5e              pop     esi
82c6d5f3 5b              pop     ebx
82c6d5f4 8be5            mov     esp,ebp
82c6d5f6 5d              pop     ebp
82c6d5f7 c21c00          ret     1Ch
lkd>
