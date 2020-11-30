fffff803`334efbe0 44884c2420      mov     byte ptr [rsp+20h],r9b
fffff803`334efbe5 4c89442418      mov     qword ptr [rsp+18h],r8
fffff803`334efbea 89542410        mov     dword ptr [rsp+10h],edx
fffff803`334efbee 53              push    rbx
fffff803`334efbef 56              push    rsi
fffff803`334efbf0 57              push    rdi
fffff803`334efbf1 4154            push    r12
fffff803`334efbf3 4155            push    r13
fffff803`334efbf5 4157            push    r15
fffff803`334efbf7 4883ec58        sub     rsp,58h
fffff803`334efbfb 654c8b3c2588010000 mov   r15,qword ptr gs:[188h]
fffff803`334efc04 488bf1          mov     rsi,rcx
fffff803`334efc07 4c8ba424b8000000 mov     r12,qword ptr [rsp+0B8h]
fffff803`334efc0f 33c9            xor     ecx,ecx
fffff803`334efc11 4c8bac24c8000000 mov     r13,qword ptr [rsp+0C8h]
fffff803`334efc19 498bd8          mov     rbx,r8
fffff803`334efc1c 888c24b8000000  mov     byte ptr [rsp+0B8h],cl
fffff803`334efc23 498bbfb8000000  mov     rdi,qword ptr [r15+0B8h]
fffff803`334efc2a 4889bc2490000000 mov     qword ptr [rsp+90h],rdi
fffff803`334efc32 49890c24        mov     qword ptr [r12],rcx
fffff803`334efc36 4d85ed          test    r13,r13
fffff803`334efc39 0f8595030000    jne     nt!ObpReferenceObjectByHandleWithTag+0x3f4 (fffff803`334effd4)

nt!ObpReferenceObjectByHandleWithTag+0x5f:
fffff803`334efc3f 488bc6          mov     rax,rsi
fffff803`334efc42 48896c2450      mov     qword ptr [rsp+50h],rbp
fffff803`334efc47 482500000080    and     rax,0FFFFFFFF80000000h
fffff803`334efc4d 4c89742448      mov     qword ptr [rsp+48h],r14
fffff803`334efc52 483d00000080    cmp     rax,0FFFFFFFF80000000h
fffff803`334efc58 0f8408020000    je      nt!ObpReferenceObjectByHandleWithTag+0x286 (fffff803`334efe66)

nt!ObpReferenceObjectByHandleWithTag+0x7e:
fffff803`334efc5e f705b8aa530000010000 test dword ptr [nt!MmVerifierData (fffff803`33a2a720)],100h
fffff803`334efc68 0f858ea21400    jne     nt!ObpReferenceObjectByHandleWithTag+0x14a31c (fffff803`33639efc)

nt!ObpReferenceObjectByHandleWithTag+0x8e:
fffff803`334efc6e 6641ff8fe4010000 dec     word ptr [r15+1E4h]
fffff803`334efc76 493bbf20020000  cmp     rdi,qword ptr [r15+220h]
fffff803`334efc7d 0f8573040000    jne     nt!ObpReferenceObjectByHandleWithTag+0x516 (fffff803`334f00f6)

nt!ObpReferenceObjectByHandleWithTag+0xa3:
fffff803`334efc83 8b8764040000    mov     eax,dword ptr [rdi+464h]
fffff803`334efc89 0fbae01a        bt      eax,1Ah
fffff803`334efc8d 0f8386a31400    jae     nt!ObpReferenceObjectByHandleWithTag+0x14a439 (fffff803`3363a019)

nt!ObpReferenceObjectByHandleWithTag+0xb3:
fffff803`334efc93 4c8b8f70050000  mov     r9,qword ptr [rdi+570h]

nt!ObpReferenceObjectByHandleWithTag+0xba:
fffff803`334efc9a 4c898c24c8000000 mov     qword ptr [rsp+0C8h],r9
fffff803`334efca2 4d85c9          test    r9,r9
fffff803`334efca5 0f846ea31400    je      nt!ObpReferenceObjectByHandleWithTag+0x14a439 (fffff803`3363a019)

nt!ObpReferenceObjectByHandleWithTag+0xcb:
fffff803`334efcab 4c3b0dce5c5300  cmp     r9,qword ptr [nt!ObpKernelHandleTable (fffff803`33a25980)]
fffff803`334efcb2 0f8437040000    je      nt!ObpReferenceObjectByHandleWithTag+0x50f (fffff803`334f00ef)

nt!ObpReferenceObjectByHandleWithTag+0xd8:
fffff803`334efcb8 f7c6fc030000    test    esi,3FCh
fffff803`334efcbe 0f8422040000    je      nt!ObpReferenceObjectByHandleWithTag+0x506 (fffff803`334f00e6)

nt!ObpReferenceObjectByHandleWithTag+0xe4:
fffff803`334efcc4 488bd6          mov     rdx,rsi
fffff803`334efcc7 498bc9          mov     rcx,r9
fffff803`334efcca e801050000      call    nt!ExpLookupHandleTableEntry (fffff803`334f01d0)
fffff803`334efccf 488bf8          mov     rdi,rax
fffff803`334efcd2 4885c0          test    rax,rax
fffff803`334efcd5 0f840b040000    je      nt!ObpReferenceObjectByHandleWithTag+0x506 (fffff803`334f00e6)

nt!ObpReferenceObjectByHandleWithTag+0xfb:
fffff803`334efcdb 0f0d08          prefetchw [rax]
fffff803`334efcde 488b08          mov     rcx,qword ptr [rax]
fffff803`334efce1 488b6808        mov     rbp,qword ptr [rax+8]
fffff803`334efce5 48896c2438      mov     qword ptr [rsp+38h],rbp
fffff803`334efcea 48894c2430      mov     qword ptr [rsp+30h],rcx
fffff803`334efcef 4c8b742430      mov     r14,qword ptr [rsp+30h]
fffff803`334efcf4 49f7c6feff0100  test    r14,1FFFEh
fffff803`334efcfb 0f84ff010000    je      nt!ObpReferenceObjectByHandleWithTag+0x320 (fffff803`334eff00)

nt!ObpReferenceObjectByHandleWithTag+0x121:
fffff803`334efd01 41f6c601        test    r14b,1
fffff803`334efd05 0f8451030000    je      nt!ObpReferenceObjectByHandleWithTag+0x47c (fffff803`334f005c)

nt!ObpReferenceObjectByHandleWithTag+0x12b:
fffff803`334efd0b 498d5efe        lea     rbx,[r14-2]
fffff803`334efd0f 488bcd          mov     rcx,rbp
fffff803`334efd12 498bc6          mov     rax,r14
fffff803`334efd15 488bd5          mov     rdx,rbp
fffff803`334efd18 f0480fc70f      lock cmpxchg16b oword ptr [rdi]
fffff803`334efd1d 4c8bf0          mov     r14,rax
fffff803`334efd20 4889442430      mov     qword ptr [rsp+30h],rax
fffff803`334efd25 488bea          mov     rbp,rdx
fffff803`334efd28 4889542438      mov     qword ptr [rsp+38h],rdx
fffff803`334efd2d 0f8558030000    jne     nt!ObpReferenceObjectByHandleWithTag+0x4ab (fffff803`334f008b)

nt!ObpReferenceObjectByHandleWithTag+0x153:
fffff803`334efd33 488bd8          mov     rbx,rax
fffff803`334efd36 48d1eb          shr     rbx,1
fffff803`334efd39 6683fb10        cmp     bx,10h
fffff803`334efd3d 0f84e4030000    je      nt!ObpReferenceObjectByHandleWithTag+0x547 (fffff803`334f0127)

nt!ObpReferenceObjectByHandleWithTag+0x163:
fffff803`334efd43 488bd8          mov     rbx,rax
fffff803`334efd46 48c1fb10        sar     rbx,10h
fffff803`334efd4a 4883e3f0        and     rbx,0FFFFFFFFFFFFFFF0h

nt!ObpReferenceObjectByHandleWithTag+0x16e:
fffff803`334efd4e 833dbbb2600000  cmp     dword ptr [nt!ObpTraceFlags (fffff803`33afb010)],0
fffff803`334efd55 0f85e2a11400    jne     nt!ObpReferenceObjectByHandleWithTag+0x14a35d (fffff803`33639f3d)

nt!ObpReferenceObjectByHandleWithTag+0x17b:
fffff803`334efd5b 488b9424a0000000 mov     rdx,qword ptr [rsp+0A0h]
fffff803`334efd63 4c8d0d960291ff  lea     r9,[nt!VrpRegistryString <PERF> (nt+0x0) (fffff803`32e00000)]
fffff803`334efd6a 488bc3          mov     rax,rbx
fffff803`334efd6d 48c1e808        shr     rax,8
fffff803`334efd71 324318          xor     al,byte ptr [rbx+18h]
fffff803`334efd74 3205a2c96000    xor     al,byte ptr [nt!ObHeaderCookie (fffff803`33afc71c)]
fffff803`334efd7a 4885d2          test    rdx,rdx
fffff803`334efd7d 0f8423010000    je      nt!ObpReferenceObjectByHandleWithTag+0x2c6 (fffff803`334efea6)

nt!ObpReferenceObjectByHandleWithTag+0x1a3:
fffff803`334efd83 384228          cmp     byte ptr [rdx+28h],al
fffff803`334efd86 0f851a010000    jne     nt!ObpReferenceObjectByHandleWithTag+0x2c6 (fffff803`334efea6)

nt!ObpReferenceObjectByHandleWithTag+0x1ac:
fffff803`334efd8c 8b8c2498000000  mov     ecx,dword ptr [rsp+98h]
fffff803`334efd93 81e5ffffff01    and     ebp,1FFFFFFh
fffff803`334efd99 80bc24a800000000 cmp     byte ptr [rsp+0A8h],0
fffff803`334efda1 7414            je      nt!ObpReferenceObjectByHandleWithTag+0x1d7 (fffff803`334efdb7)

nt!ObpReferenceObjectByHandleWithTag+0x1c3:
fffff803`334efda3 8bc5            mov     eax,ebp
fffff803`334efda5 f7d0            not     eax
fffff803`334efda7 85c1            test    ecx,eax
fffff803`334efda9 0f85ee020000    jne     nt!ObpReferenceObjectByHandleWithTag+0x4bd (fffff803`334f009d)

nt!ObpReferenceObjectByHandleWithTag+0x1cf:
fffff803`334efdaf 0fb6431a        movzx   eax,byte ptr [rbx+1Ah]
fffff803`334efdb3 a840            test    al,40h
fffff803`334efdb5 7575            jne     nt!ObpReferenceObjectByHandleWithTag+0x24c (fffff803`334efe2c)

nt!ObpReferenceObjectByHandleWithTag+0x1d7:
fffff803`334efdb7 488b8424c0000000 mov     rax,qword ptr [rsp+0C0h]
fffff803`334efdbf 49c1fe11        sar     r14,11h
fffff803`334efdc3 4885c0          test    rax,rax
fffff803`334efdc6 0f850e010000    jne     nt!ObpReferenceObjectByHandleWithTag+0x2fa (fffff803`334efeda)

nt!ObpReferenceObjectByHandleWithTag+0x1ec:
fffff803`334efdcc 41f6c604        test    r14b,4
fffff803`334efdd0 0f8591a11400    jne     nt!ObpReferenceObjectByHandleWithTag+0x14a387 (fffff803`33639f67)

nt!ObpReferenceObjectByHandleWithTag+0x1f6:
fffff803`334efdd6 4532d2          xor     r10b,r10b

nt!ObpReferenceObjectByHandleWithTag+0x1f9:
fffff803`334efdd9 4c8b9c24c8000000 mov     r11,qword ptr [rsp+0C8h]
fffff803`334efde1 4d85ed          test    r13,r13
fffff803`334efde4 0f85f5010000    jne     nt!ObpReferenceObjectByHandleWithTag+0x3ff (fffff803`334effdf)

nt!ObpReferenceObjectByHandleWithTag+0x20a:
fffff803`334efdea 4584d2          test    r10b,r10b
fffff803`334efded 0f85a3a11400    jne     nt!ObpReferenceObjectByHandleWithTag+0x14a3b6 (fffff803`33639f96)

nt!ObpReferenceObjectByHandleWithTag+0x213:
fffff803`334efdf3 80bc24b800000000 cmp     byte ptr [rsp+0B8h],0
fffff803`334efdfb 488d4330        lea     rax,[rbx+30h]
fffff803`334efdff 49890424        mov     qword ptr [r12],rax
fffff803`334efe03 0f8505030000    jne     nt!ObpReferenceObjectByHandleWithTag+0x52e (fffff803`334f010e)

nt!ObpReferenceObjectByHandleWithTag+0x229:
fffff803`334efe09 498bcf          mov     rcx,r15
fffff803`334efe0c e84f31bdff      call    nt!KeLeaveCriticalRegionThread (fffff803`330c2f60)
fffff803`334efe11 33c0            xor     eax,eax

nt!ObpReferenceObjectByHandleWithTag+0x233:
fffff803`334efe13 4c8b742448      mov     r14,qword ptr [rsp+48h]
fffff803`334efe18 488b6c2450      mov     rbp,qword ptr [rsp+50h]
fffff803`334efe1d 4883c458        add     rsp,58h
fffff803`334efe21 415f            pop     r15
fffff803`334efe23 415d            pop     r13
fffff803`334efe25 415c            pop     r12
fffff803`334efe27 5f              pop     rdi
fffff803`334efe28 5e              pop     rsi
fffff803`334efe29 5b              pop     rbx
fffff803`334efe2a c3              ret

nt!ObpReferenceObjectByHandleWithTag+0x24c:
fffff803`334efe2c 83e07f          and     eax,7Fh
fffff803`334efe2f 488bcb          mov     rcx,rbx
fffff803`334efe32 420fb68408805ec200 movzx eax,byte ptr [rax+r9+0C25E80h]
fffff803`334efe3b 482bc8          sub     rcx,rax
fffff803`334efe3e 488b01          mov     rax,qword ptr [rcx]
fffff803`334efe41 80781800        cmp     byte ptr [rax+18h],0
fffff803`334efe45 7413            je      nt!ObpReferenceObjectByHandleWithTag+0x27a (fffff803`334efe5a)

nt!ObpReferenceObjectByHandleWithTag+0x267:
fffff803`334efe47 488b4010        mov     rax,qword ptr [rax+10h]
fffff803`334efe4b 4883f801        cmp     rax,1
fffff803`334efe4f 0f8408a11400    je      nt!ObpReferenceObjectByHandleWithTag+0x14a37d (fffff803`33639f5d)

nt!ObpReferenceObjectByHandleWithTag+0x275:
fffff803`334efe55 4c8b742430      mov     r14,qword ptr [rsp+30h]

nt!ObpReferenceObjectByHandleWithTag+0x27a:
fffff803`334efe5a 8b8c2498000000  mov     ecx,dword ptr [rsp+98h]
fffff803`334efe61 e951ffffff      jmp     nt!ObpReferenceObjectByHandleWithTag+0x1d7 (fffff803`334efdb7)

nt!ObpReferenceObjectByHandleWithTag+0x286:
fffff803`334efe66 4883feff        cmp     rsi,0FFFFFFFFFFFFFFFFh
fffff803`334efe6a 0f847f010000    je      nt!ObpReferenceObjectByHandleWithTag+0x40f (fffff803`334effef)

nt!ObpReferenceObjectByHandleWithTag+0x290:
fffff803`334efe70 4883fefe        cmp     rsi,0FFFFFFFFFFFFFFFEh
fffff803`334efe74 0f8401010000    je      nt!ObpReferenceObjectByHandleWithTag+0x39b (fffff803`334eff7b)

nt!ObpReferenceObjectByHandleWithTag+0x29a:
fffff803`334efe7a 4584c9          test    r9b,r9b
fffff803`334efe7d 0f856fa01400    jne     nt!ObpReferenceObjectByHandleWithTag+0x14a312 (fffff803`33639ef2)

nt!ObpReferenceObjectByHandleWithTag+0x2a3:
fffff803`334efe83 4c8b0df65a5300  mov     r9,qword ptr [nt!ObpKernelHandleTable (fffff803`33a25980)]
fffff803`334efe8a 4881f600000080  xor     rsi,0FFFFFFFF80000000h
fffff803`334efe91 6641ff8fe4010000 dec     word ptr [r15+1E4h]
fffff803`334efe99 4c898c24c8000000 mov     qword ptr [rsp+0C8h],r9
fffff803`334efea1 e912feffff      jmp     nt!ObpReferenceObjectByHandleWithTag+0xd8 (fffff803`334efcb8)

nt!ObpReferenceObjectByHandleWithTag+0x2c6:
fffff803`334efea6 0fb6c8          movzx   ecx,al
fffff803`334efea9 4d8b84c910cecf00 mov     r8,qword ptr [r9+rcx*8+0CFCE10h]
fffff803`334efeb1 4d85c0          test    r8,r8
fffff803`334efeb4 0f840ba11400    je      nt!ObpReferenceObjectByHandleWithTag+0x14a3e5 (fffff803`33639fc5)

nt!ObpReferenceObjectByHandleWithTag+0x2da:
fffff803`334efeba 4c3b0577c66000  cmp     r8,qword ptr [nt!MmBadPointer (fffff803`33afc538)]
fffff803`334efec1 0f84fea01400    je      nt!ObpReferenceObjectByHandleWithTag+0x14a3e5 (fffff803`33639fc5)

nt!ObpReferenceObjectByHandleWithTag+0x2e7:
fffff803`334efec7 4885d2          test    rdx,rdx
fffff803`334efeca 0f84bcfeffff    je      nt!ObpReferenceObjectByHandleWithTag+0x1ac (fffff803`334efd8c)

nt!ObpReferenceObjectByHandleWithTag+0x2f0:
fffff803`334efed0 bf240000c0      mov     edi,0C0000024h
fffff803`334efed5 e9c8010000      jmp     nt!ObpReferenceObjectByHandleWithTag+0x4c2 (fffff803`334f00a2)

nt!ObpReferenceObjectByHandleWithTag+0x2fa:
fffff803`334efeda 4183e607        and     r14d,7
fffff803`334efede 896804          mov     dword ptr [rax+4],ebp
fffff803`334efee1 448930          mov     dword ptr [rax],r14d
fffff803`334efee4 41f6c604        test    r14b,4
fffff803`334efee8 0f84e8feffff    je      nt!ObpReferenceObjectByHandleWithTag+0x1f6 (fffff803`334efdd6)

nt!ObpReferenceObjectByHandleWithTag+0x30e:
fffff803`334efeee e974a01400      jmp     nt!ObpReferenceObjectByHandleWithTag+0x14a387 (fffff803`33639f67)

nt!ObpReferenceObjectByHandleWithTag+0x320:
fffff803`334eff00 0f0d0f          prefetchw [rdi]
fffff803`334eff03 4c8b07          mov     r8,qword ptr [rdi]
fffff803`334eff06 41f6c001        test    r8b,1
fffff803`334eff0a 0f84cd010000    je      nt!ObpReferenceObjectByHandleWithTag+0x4fd (fffff803`334f00dd)

nt!ObpReferenceObjectByHandleWithTag+0x330:
fffff803`334eff10 498d48ff        lea     rcx,[r8-1]
fffff803`334eff14 498bc0          mov     rax,r8
fffff803`334eff17 f0480fb10f      lock cmpxchg qword ptr [rdi],rcx
fffff803`334eff1c 75e2            jne     nt!ObpReferenceObjectByHandleWithTag+0x320 (fffff803`334eff00)

nt!ObpReferenceObjectByHandleWithTag+0x33e:
fffff803`334eff1e 488b1f          mov     rbx,qword ptr [rdi]
fffff803`334eff21 488bcf          mov     rcx,rdi
fffff803`334eff24 0f1007          movups  xmm0,xmmword ptr [rdi]
fffff803`334eff27 48c1fb10        sar     rbx,10h
fffff803`334eff2b 4883e3f0        and     rbx,0FFFFFFFFFFFFFFF0h
fffff803`334eff2f 0f11442430      movups  xmmword ptr [rsp+30h],xmm0
fffff803`334eff34 e837ebbeff      call    nt!ExSlowReplenishHandleTableEntry (fffff803`330dea70)
fffff803`334eff39 ffc0            inc     eax
fffff803`334eff3b 4863c8          movsxd  rcx,eax
fffff803`334eff3e 488bc1          mov     rax,rcx
fffff803`334eff41 f0480fc103      lock xadd qword ptr [rbx],rax
fffff803`334eff46 4885c0          test    rax,rax
fffff803`334eff49 0f8ed39f1400    jle     nt!ObpReferenceObjectByHandleWithTag+0x14a342 (fffff803`33639f22)

nt!ObpReferenceObjectByHandleWithTag+0x36f:
fffff803`334eff4f b801000000      mov     eax,1
fffff803`334eff54 f0480fc107      lock xadd qword ptr [rdi],rax
fffff803`334eff59 498d4930        lea     rcx,[r9+30h]
fffff803`334eff5d f0830c2400      lock or dword ptr [rsp],0
fffff803`334eff62 48833900        cmp     qword ptr [rcx],0
fffff803`334eff66 0f8539020000    jne     nt!ObpReferenceObjectByHandleWithTag+0x5c5 (fffff803`334f01a5)

nt!ObpReferenceObjectByHandleWithTag+0x38c:
fffff803`334eff6c 488b6c2438      mov     rbp,qword ptr [rsp+38h]
fffff803`334eff71 4c8b742430      mov     r14,qword ptr [rsp+30h]
fffff803`334eff76 e9d3fdffff      jmp     nt!ObpReferenceObjectByHandleWithTag+0x16e (fffff803`334efd4e)

nt!ObpReferenceObjectByHandleWithTag+0x39b:
fffff803`334eff7b 483b1dbec46000  cmp     rbx,qword ptr [nt!PsThreadType (fffff803`33afc440)]
fffff803`334eff82 0f85f0010000    jne     nt!ObpReferenceObjectByHandleWithTag+0x598 (fffff803`334f0178)

nt!ObpReferenceObjectByHandleWithTag+0x3a8:
fffff803`334eff88 f7c20000e0ff    test    edx,0FFE00000h
fffff803`334eff8e 0f85149f1400    jne     nt!ObpReferenceObjectByHandleWithTag+0x14a2c8 (fffff803`33639ea8)

nt!ObpReferenceObjectByHandleWithTag+0x3b4:
fffff803`334eff94 488b8424c0000000 mov     rax,qword ptr [rsp+0C0h]
fffff803`334eff9c 4885c0          test    rax,rax
fffff803`334eff9f 0f850c020000    jne     nt!ObpReferenceObjectByHandleWithTag+0x5d1 (fffff803`334f01b1)

nt!ObpReferenceObjectByHandleWithTag+0x3c5:
fffff803`334effa5 390d65b06000    cmp     dword ptr [nt!ObpTraceFlags (fffff803`33afb010)],ecx
fffff803`334effab bb01000000      mov     ebx,1
fffff803`334effb0 0f85059f1400    jne     nt!ObpReferenceObjectByHandleWithTag+0x14a2db (fffff803`33639ebb)

nt!ObpReferenceObjectByHandleWithTag+0x3d6:
fffff803`334effb6 f0490fc15fd0    lock xadd qword ptr [r15-30h],rbx
fffff803`334effbc 48ffc3          inc     rbx
fffff803`334effbf 4883fb01        cmp     rbx,1
fffff803`334effc3 0f8e109f1400    jle     nt!ObpReferenceObjectByHandleWithTag+0x14a2f9 (fffff803`33639ed9)

nt!ObpReferenceObjectByHandleWithTag+0x3e9:
fffff803`334effc9 4d893c24        mov     qword ptr [r12],r15

nt!ObpReferenceObjectByHandleWithTag+0x3ed:
fffff803`334effcd 8bc1            mov     eax,ecx
fffff803`334effcf e93ffeffff      jmp     nt!ObpReferenceObjectByHandleWithTag+0x233 (fffff803`334efe13)

nt!ObpReferenceObjectByHandleWithTag+0x3f4:
fffff803`334effd4 33c0            xor     eax,eax
fffff803`334effd6 49894500        mov     qword ptr [r13],rax
fffff803`334effda e960fcffff      jmp     nt!ObpReferenceObjectByHandleWithTag+0x5f (fffff803`334efc3f)

nt!ObpReferenceObjectByHandleWithTag+0x3ff:
fffff803`334effdf 41837b0400      cmp     dword ptr [r11+4],0
fffff803`334effe4 0f8400feffff    je      nt!ObpReferenceObjectByHandleWithTag+0x20a (fffff803`334efdea)

nt!ObpReferenceObjectByHandleWithTag+0x40a:
fffff803`334effea e9809f1400      jmp     nt!ObpReferenceObjectByHandleWithTag+0x14a38f (fffff803`33639f6f)

nt!ObpReferenceObjectByHandleWithTag+0x40f:
fffff803`334effef 483b1d1ac46000  cmp     rbx,qword ptr [nt!PsProcessType (fffff803`33afc410)]
fffff803`334efff6 7553            jne     nt!ObpReferenceObjectByHandleWithTag+0x46b (fffff803`334f004b)

nt!ObpReferenceObjectByHandleWithTag+0x418:
fffff803`334efff8 498bbfb8000000  mov     rdi,qword ptr [r15+0B8h]
fffff803`334effff f7c20000e0ff    test    edx,0FFE00000h
fffff803`334f0005 0f85539e1400    jne     nt!ObpReferenceObjectByHandleWithTag+0x14a27e (fffff803`33639e5e)

nt!ObpReferenceObjectByHandleWithTag+0x42b:
fffff803`334f000b 488b8424c0000000 mov     rax,qword ptr [rsp+0C0h]
fffff803`334f0013 4885c0          test    rax,rax
fffff803`334f0016 0f85b3000000    jne     nt!ObpReferenceObjectByHandleWithTag+0x4ef (fffff803`334f00cf)

nt!ObpReferenceObjectByHandleWithTag+0x43c:
fffff803`334f001c 390deeaf6000    cmp     dword ptr [nt!ObpTraceFlags (fffff803`33afb010)],ecx
fffff803`334f0022 bb01000000      mov     ebx,1
fffff803`334f0027 0f85449e1400    jne     nt!ObpReferenceObjectByHandleWithTag+0x14a291 (fffff803`33639e71)

nt!ObpReferenceObjectByHandleWithTag+0x44d:
fffff803`334f002d f0480fc15fd0    lock xadd qword ptr [rdi-30h],rbx
fffff803`334f0033 48ffc3          inc     rbx
fffff803`334f0036 4883fb01        cmp     rbx,1
fffff803`334f003a 0f8e4f9e1400    jle     nt!ObpReferenceObjectByHandleWithTag+0x14a2af (fffff803`33639e8f)

nt!ObpReferenceObjectByHandleWithTag+0x460:
fffff803`334f0040 49893c24        mov     qword ptr [r12],rdi

nt!ObpReferenceObjectByHandleWithTag+0x464:
fffff803`334f0044 8bc1            mov     eax,ecx
fffff803`334f0046 e9c8fdffff      jmp     nt!ObpReferenceObjectByHandleWithTag+0x233 (fffff803`334efe13)

nt!ObpReferenceObjectByHandleWithTag+0x46b:
fffff803`334f004b 4885db          test    rbx,rbx
fffff803`334f004e 74a8            je      nt!ObpReferenceObjectByHandleWithTag+0x418 (fffff803`334efff8)

nt!ObpReferenceObjectByHandleWithTag+0x470:
fffff803`334f0050 b9240000c0      mov     ecx,0C0000024h
fffff803`334f0055 8bc1            mov     eax,ecx
fffff803`334f0057 e9b7fdffff      jmp     nt!ObpReferenceObjectByHandleWithTag+0x233 (fffff803`334efe13)

nt!ObpReferenceObjectByHandleWithTag+0x47c:
fffff803`334f005c 4d8bc6          mov     r8,r14
fffff803`334f005f 488bd7          mov     rdx,rdi
fffff803`334f0062 498bc9          mov     rcx,r9
fffff803`334f0065 e8224df6ff      call    nt!ExpBlockOnLockedHandleEntry (fffff803`33454d8c)
fffff803`334f006a 0f0d0f          prefetchw [rdi]
fffff803`334f006d 488b07          mov     rax,qword ptr [rdi]
fffff803`334f0070 488b6f08        mov     rbp,qword ptr [rdi+8]
fffff803`334f0074 4c8b8c24c8000000 mov     r9,qword ptr [rsp+0C8h]
fffff803`334f007c 4889442430      mov     qword ptr [rsp+30h],rax
fffff803`334f0081 4c8b742430      mov     r14,qword ptr [rsp+30h]
fffff803`334f0086 48896c2438      mov     qword ptr [rsp+38h],rbp

nt!ObpReferenceObjectByHandleWithTag+0x4ab:
fffff803`334f008b 49f7c6feff0100  test    r14,1FFFEh
fffff803`334f0092 0f8569fcffff    jne     nt!ObpReferenceObjectByHandleWithTag+0x121 (fffff803`334efd01)

nt!ObpReferenceObjectByHandleWithTag+0x4b8:
fffff803`334f0098 e963feffff      jmp     nt!ObpReferenceObjectByHandleWithTag+0x320 (fffff803`334eff00)

nt!ObpReferenceObjectByHandleWithTag+0x4bd:
fffff803`334f009d bf220000c0      mov     edi,0C0000022h

nt!ObpReferenceObjectByHandleWithTag+0x4c2:
fffff803`334f00a2 8b9424b0000000  mov     edx,dword ptr [rsp+0B0h]
fffff803`334f00a9 488d4b30        lea     rcx,[rbx+30h]
fffff803`334f00ad e82eb9beff      call    nt!ObfDereferenceObjectWithTag (fffff803`330db9e0)

nt!ObpReferenceObjectByHandleWithTag+0x4d2:
fffff803`334f00b2 80bc24b800000000 cmp     byte ptr [rsp+0B8h],0
fffff803`334f00ba 0f853f9f1400    jne     nt!ObpReferenceObjectByHandleWithTag+0x14a41f (fffff803`33639fff)

nt!ObpReferenceObjectByHandleWithTag+0x4e0:
fffff803`334f00c0 498bcf          mov     rcx,r15
fffff803`334f00c3 e8982ebdff      call    nt!KeLeaveCriticalRegionThread (fffff803`330c2f60)
fffff803`334f00c8 8bc7            mov     eax,edi
fffff803`334f00ca e944fdffff      jmp     nt!ObpReferenceObjectByHandleWithTag+0x233 (fffff803`334efe13)

nt!ObpReferenceObjectByHandleWithTag+0x4ef:
fffff803`334f00cf c74004ffff1f00  mov     dword ptr [rax+4],1FFFFFh
fffff803`334f00d6 8908            mov     dword ptr [rax],ecx
fffff803`334f00d8 e93fffffff      jmp     nt!ObpReferenceObjectByHandleWithTag+0x43c (fffff803`334f001c)

nt!ObpReferenceObjectByHandleWithTag+0x4fd:
fffff803`334f00dd 4d85c0          test    r8,r8
fffff803`334f00e0 0f85a7000000    jne     nt!ObpReferenceObjectByHandleWithTag+0x5ad (fffff803`334f018d)

nt!ObpReferenceObjectByHandleWithTag+0x506:
fffff803`334f00e6 4885f6          test    rsi,rsi
fffff803`334f00e9 0f85ee9e1400    jne     nt!ObpReferenceObjectByHandleWithTag+0x14a3fd (fffff803`33639fdd)

nt!ObpReferenceObjectByHandleWithTag+0x50f:
fffff803`334f00ef bf080000c0      mov     edi,0C0000008h
fffff803`334f00f4 ebbc            jmp     nt!ObpReferenceObjectByHandleWithTag+0x4d2 (fffff803`334f00b2)

nt!ObpReferenceObjectByHandleWithTag+0x516:
fffff803`334f00f6 488bcf          mov     rcx,rdi
fffff803`334f00f9 e8663cffff      call    nt!ObReferenceProcessHandleTable (fffff803`334e3d64)
fffff803`334f00fe 4c8bc8          mov     r9,rax
fffff803`334f0101 c68424b800000001 mov     byte ptr [rsp+0B8h],1
fffff803`334f0109 e98cfbffff      jmp     nt!ObpReferenceObjectByHandleWithTag+0xba (fffff803`334efc9a)

nt!ObpReferenceObjectByHandleWithTag+0x52e:
fffff803`334f010e 488b8c2490000000 mov     rcx,qword ptr [rsp+90h]
fffff803`334f0116 4881c158040000  add     rcx,458h
fffff803`334f011d e87e3fbfff      call    nt!ExReleaseRundownProtection (fffff803`330e40a0)
fffff803`334f0122 e9e2fcffff      jmp     nt!ObpReferenceObjectByHandleWithTag+0x229 (fffff803`334efe09)

nt!ObpReferenceObjectByHandleWithTag+0x547:
fffff803`334f0127 488d1c5dfeffffff lea     rbx,[rbx*2-2]
fffff803`334f012f baf07f0000      mov     edx,7FF0h
fffff803`334f0134 4833d8          xor     rbx,rax
fffff803`334f0137 81e3feff0100    and     ebx,1FFFEh
fffff803`334f013d 4833d8          xor     rbx,rax
fffff803`334f0140 48895c2430      mov     qword ptr [rsp+30h],rbx
fffff803`334f0145 48c1fb10        sar     rbx,10h
fffff803`334f0149 4883e3f0        and     rbx,0FFFFFFFFFFFFFFF0h
fffff803`334f014d 488bcb          mov     rcx,rbx
fffff803`334f0150 e87371baff      call    nt!ObpIncrPointerCountEx (fffff803`330972c8)
fffff803`334f0155 41b8f07f0000    mov     r8d,7FF0h
fffff803`334f015b 488d542430      lea     rdx,[rsp+30h]
fffff803`334f0160 488bcf          mov     rcx,rdi
fffff803`334f0163 e8684abbff      call    nt!ExFastReplenishHandleTableEntry (fffff803`330a4bd0)
fffff803`334f0168 4863c8          movsxd  rcx,eax
fffff803`334f016b 85c0            test    eax,eax
fffff803`334f016d 0f84f9fdffff    je      nt!ObpReferenceObjectByHandleWithTag+0x38c (fffff803`334eff6c)

nt!ObpReferenceObjectByHandleWithTag+0x593:
fffff803`334f0173 e99b9d1400      jmp     nt!ObpReferenceObjectByHandleWithTag+0x14a333 (fffff803`33639f13)

nt!ObpReferenceObjectByHandleWithTag+0x598:
fffff803`334f0178 4885db          test    rbx,rbx
fffff803`334f017b 0f8407feffff    je      nt!ObpReferenceObjectByHandleWithTag+0x3a8 (fffff803`334eff88)

nt!ObpReferenceObjectByHandleWithTag+0x5a1:
fffff803`334f0181 b9240000c0      mov     ecx,0C0000024h
fffff803`334f0186 8bc1            mov     eax,ecx
fffff803`334f0188 e986fcffff      jmp     nt!ObpReferenceObjectByHandleWithTag+0x233 (fffff803`334efe13)

nt!ObpReferenceObjectByHandleWithTag+0x5ad:
fffff803`334f018d 488bd7          mov     rdx,rdi
fffff803`334f0190 498bc9          mov     rcx,r9
fffff803`334f0193 e8f44bf6ff      call    nt!ExpBlockOnLockedHandleEntry (fffff803`33454d8c)
fffff803`334f0198 4c8b8c24c8000000 mov     r9,qword ptr [rsp+0C8h]
fffff803`334f01a0 e95bfdffff      jmp     nt!ObpReferenceObjectByHandleWithTag+0x320 (fffff803`334eff00)

nt!ObpReferenceObjectByHandleWithTag+0x5c5:
fffff803`334f01a5 33d2            xor     edx,edx
fffff803`334f01a7 e8e4ffcfff      call    nt!ExfUnblockPushLock (fffff803`331f0190)
fffff803`334f01ac e9bbfdffff      jmp     nt!ObpReferenceObjectByHandleWithTag+0x38c (fffff803`334eff6c)

nt!ObpReferenceObjectByHandleWithTag+0x5d1:
fffff803`334f01b1 c74004ffff1f00  mov     dword ptr [rax+4],1FFFFFh
fffff803`334f01b8 8908            mov     dword ptr [rax],ecx
fffff803`334f01ba e9e6fdffff      jmp     nt!ObpReferenceObjectByHandleWithTag+0x3c5 (fffff803`334effa5)

nt!ObpReferenceObjectByHandleWithTag+0x14a27e:
fffff803`33639e5e 4584c9          test    r9b,r9b
fffff803`33639e61 0f84a461ebff    je      nt!ObpReferenceObjectByHandleWithTag+0x42b (fffff803`334f000b)

nt!ObpReferenceObjectByHandleWithTag+0x14a287:
fffff803`33639e67 b9220000c0      mov     ecx,0C0000022h
fffff803`33639e6c e9d361ebff      jmp     nt!ObpReferenceObjectByHandleWithTag+0x464 (fffff803`334f0044)

nt!ObpReferenceObjectByHandleWithTag+0x14a291:
fffff803`33639e71 448b8c24b0000000 mov     r9d,dword ptr [rsp+0B0h]
fffff803`33639e79 488d4fd0        lea     rcx,[rdi-30h]
fffff803`33639e7d 448bc3          mov     r8d,ebx
fffff803`33639e80 0fb6d3          movzx   edx,bl
fffff803`33639e83 e8004ad2ff      call    nt!ObpPushStackInfo (fffff803`3335e888)
fffff803`33639e88 33c9            xor     ecx,ecx
fffff803`33639e8a e99e61ebff      jmp     nt!ObpReferenceObjectByHandleWithTag+0x44d (fffff803`334f002d)

nt!ObpReferenceObjectByHandleWithTag+0x14a2af:
fffff803`33639e8f 33d2            xor     edx,edx
fffff803`33639e91 48895c2420      mov     qword ptr [rsp+20h],rbx
fffff803`33639e96 41b910000000    mov     r9d,10h
fffff803`33639e9c 4c8bc7          mov     r8,rdi
fffff803`33639e9f 8d4a18          lea     ecx,[rdx+18h]
fffff803`33639ea2 e8f9a6bbff      call    nt!KeBugCheckEx (fffff803`331f45a0)
fffff803`33639ea7 cc              int     3

nt!ObpReferenceObjectByHandleWithTag+0x14a2c8:
fffff803`33639ea8 4584c9          test    r9b,r9b
fffff803`33639eab 0f84e360ebff    je      nt!ObpReferenceObjectByHandleWithTag+0x3b4 (fffff803`334eff94)

nt!ObpReferenceObjectByHandleWithTag+0x14a2d1:
fffff803`33639eb1 b9220000c0      mov     ecx,0C0000022h
fffff803`33639eb6 e91261ebff      jmp     nt!ObpReferenceObjectByHandleWithTag+0x3ed (fffff803`334effcd)

nt!ObpReferenceObjectByHandleWithTag+0x14a2db:
fffff803`33639ebb 448b8c24b0000000 mov     r9d,dword ptr [rsp+0B0h]
fffff803`33639ec3 498d4fd0        lea     rcx,[r15-30h]
fffff803`33639ec7 448bc3          mov     r8d,ebx
fffff803`33639eca 0fb6d3          movzx   edx,bl
fffff803`33639ecd e8b649d2ff      call    nt!ObpPushStackInfo (fffff803`3335e888)
fffff803`33639ed2 33c9            xor     ecx,ecx
fffff803`33639ed4 e9dd60ebff      jmp     nt!ObpReferenceObjectByHandleWithTag+0x3d6 (fffff803`334effb6)

nt!ObpReferenceObjectByHandleWithTag+0x14a2f9:
fffff803`33639ed9 33d2            xor     edx,edx
fffff803`33639edb 48895c2420      mov     qword ptr [rsp+20h],rbx
fffff803`33639ee0 41b910000000    mov     r9d,10h
fffff803`33639ee6 4d8bc7          mov     r8,r15
fffff803`33639ee9 8d4a18          lea     ecx,[rdx+18h]
fffff803`33639eec e8afa6bbff      call    nt!KeBugCheckEx (fffff803`331f45a0)
fffff803`33639ef1 cc              int     3

nt!ObpReferenceObjectByHandleWithTag+0x14a312:
fffff803`33639ef2 b8080000c0      mov     eax,0C0000008h
fffff803`33639ef7 e9175febff      jmp     nt!ObpReferenceObjectByHandleWithTag+0x233 (fffff803`334efe13)

nt!ObpReferenceObjectByHandleWithTag+0x14a31c:
fffff803`33639efc 4584c9          test    r9b,r9b
fffff803`33639eff 0f85695debff    jne     nt!ObpReferenceObjectByHandleWithTag+0x8e (fffff803`334efc6e)

nt!ObpReferenceObjectByHandleWithTag+0x14a325:
fffff803`33639f05 488bce          mov     rcx,rsi
fffff803`33639f08 e827d21900      call    nt!VfCheckUserHandle (fffff803`337d7134)
fffff803`33639f0d 90              nop
fffff803`33639f0e e95b5debff      jmp     nt!ObpReferenceObjectByHandleWithTag+0x8e (fffff803`334efc6e)

nt!ObpReferenceObjectByHandleWithTag+0x14a333:
fffff803`33639f13 f7d9            neg     ecx
fffff803`33639f15 4863c1          movsxd  rax,ecx
fffff803`33639f18 f0480fc103      lock xadd qword ptr [rbx],rax
fffff803`33639f1d e94a60ebff      jmp     nt!ObpReferenceObjectByHandleWithTag+0x38c (fffff803`334eff6c)

nt!ObpReferenceObjectByHandleWithTag+0x14a342:
fffff803`33639f22 33d2            xor     edx,edx
fffff803`33639f24 4c8d4330        lea     r8,[rbx+30h]
fffff803`33639f28 4803c1          add     rax,rcx
fffff803`33639f2b 4889442420      mov     qword ptr [rsp+20h],rax
fffff803`33639f30 8d4a18          lea     ecx,[rdx+18h]
fffff803`33639f33 448d4a10        lea     r9d,[rdx+10h]
fffff803`33639f37 e864a6bbff      call    nt!KeBugCheckEx (fffff803`331f45a0)
fffff803`33639f3c cc              int     3

nt!ObpReferenceObjectByHandleWithTag+0x14a35d:
fffff803`33639f3d 448b8c24b0000000 mov     r9d,dword ptr [rsp+0B0h]
fffff803`33639f45 41b801000000    mov     r8d,1
fffff803`33639f4b 410fb6d0        movzx   edx,r8b
fffff803`33639f4f 488bcb          mov     rcx,rbx
fffff803`33639f52 e83149d2ff      call    nt!ObpPushStackInfo (fffff803`3335e888)
fffff803`33639f57 90              nop
fffff803`33639f58 e9fe5debff      jmp     nt!ObpReferenceObjectByHandleWithTag+0x17b (fffff803`334efd5b)

nt!ObpReferenceObjectByHandleWithTag+0x14a37d:
fffff803`33639f5d bf06a000c0      mov     edi,0C000A006h
fffff803`33639f62 e93b61ebff      jmp     nt!ObpReferenceObjectByHandleWithTag+0x4c2 (fffff803`334f00a2)

nt!ObpReferenceObjectByHandleWithTag+0x14a387:
fffff803`33639f67 41b201          mov     r10b,1
fffff803`33639f6a e96a5eebff      jmp     nt!ObpReferenceObjectByHandleWithTag+0x1f9 (fffff803`334efdd9)

nt!ObpReferenceObjectByHandleWithTag+0x14a38f:
fffff803`33639f6f 488bd6          mov     rdx,rsi
fffff803`33639f72 498bcb          mov     rcx,r11
fffff803`33639f75 e856e81000      call    nt!ExpGetHandleExtraInfo (fffff803`337487d0)
fffff803`33639f7a 8b8c2498000000  mov     ecx,dword ptr [rsp+98h]
fffff803`33639f81 4885c0          test    rax,rax
fffff803`33639f84 0f84605eebff    je      nt!ObpReferenceObjectByHandleWithTag+0x20a (fffff803`334efdea)

nt!ObpReferenceObjectByHandleWithTag+0x14a3aa:
fffff803`33639f8a 488b00          mov     rax,qword ptr [rax]
fffff803`33639f8d 49894500        mov     qword ptr [r13],rax
fffff803`33639f91 e9545eebff      jmp     nt!ObpReferenceObjectByHandleWithTag+0x20a (fffff803`334efdea)

nt!ObpReferenceObjectByHandleWithTag+0x14a3b6:
fffff803`33639f96 85c9            test    ecx,ecx
fffff803`33639f98 0f84555eebff    je      nt!ObpReferenceObjectByHandleWithTag+0x213 (fffff803`334efdf3)

nt!ObpReferenceObjectByHandleWithTag+0x14a3be:
fffff803`33639f9e 894c2420        mov     dword ptr [rsp+20h],ecx
fffff803`33639fa2 4c8bcb          mov     r9,rbx
fffff803`33639fa5 498bcb          mov     rcx,r11
fffff803`33639fa8 4c8bc7          mov     r8,rdi
fffff803`33639fab 488bd6          mov     rdx,rsi
fffff803`33639fae e8e1e80900      call    nt!ObpAuditObjectAccess (fffff803`336d8894)
fffff803`33639fb3 84c0            test    al,al
fffff803`33639fb5 0f85385eebff    jne     nt!ObpReferenceObjectByHandleWithTag+0x213 (fffff803`334efdf3)

nt!ObpReferenceObjectByHandleWithTag+0x14a3db:
fffff803`33639fbb bf080000c0      mov     edi,0C0000008h
fffff803`33639fc0 e9dd60ebff      jmp     nt!ObpReferenceObjectByHandleWithTag+0x4c2 (fffff803`334f00a2)

nt!ObpReferenceObjectByHandleWithTag+0x14a3e5:
fffff803`33639fc5 33ff            xor     edi,edi
fffff803`33639fc7 4533c9          xor     r9d,r9d
fffff803`33639fca 488bd3          mov     rdx,rbx
fffff803`33639fcd 48897c2420      mov     qword ptr [rsp+20h],rdi
fffff803`33639fd2 b989010000      mov     ecx,189h
fffff803`33639fd7 e8c4a5bbff      call    nt!KeBugCheckEx (fffff803`331f45a0)
fffff803`33639fdc cc              int     3

nt!ObpReferenceObjectByHandleWithTag+0x14a3fd:
fffff803`33639fdd 65488b042588010000 mov   rax,qword ptr gs:[188h]
fffff803`33639fe6 488bd6          mov     rdx,rsi
fffff803`33639fe9 498bc9          mov     rcx,r9
fffff803`33639fec 440fb68032020000 movzx   r8d,byte ptr [rax+232h]
fffff803`33639ff4 e8bf719cff      call    nt!ExHandleLogBadReference (fffff803`330011b8)
fffff803`33639ff9 90              nop
fffff803`33639ffa e9f060ebff      jmp     nt!ObpReferenceObjectByHandleWithTag+0x50f (fffff803`334f00ef)

nt!ObpReferenceObjectByHandleWithTag+0x14a41f:
fffff803`33639fff 488b8c2490000000 mov     rcx,qword ptr [rsp+90h]
fffff803`3363a007 4881c158040000  add     rcx,458h
fffff803`3363a00e e88da0aaff      call    nt!ExReleaseRundownProtection (fffff803`330e40a0)
fffff803`3363a013 90              nop
fffff803`3363a014 e9a760ebff      jmp     nt!ObpReferenceObjectByHandleWithTag+0x4e0 (fffff803`334f00c0)

nt!ObpReferenceObjectByHandleWithTag+0x14a439:
fffff803`3363a019 bf080000c0      mov     edi,0C0000008h
fffff803`3363a01e e99d60ebff      jmp     nt!ObpReferenceObjectByHandleWithTag+0x4e0 (fffff803`334f00c0)