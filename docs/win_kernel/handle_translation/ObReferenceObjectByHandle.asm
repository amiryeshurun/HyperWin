fffff803`334efba0 4883ec48        sub     rsp,48h
fffff803`334efba4 488b442478      mov     rax,qword ptr [rsp+78h]
fffff803`334efba9 48c744243800000000 mov   qword ptr [rsp+38h],0
fffff803`334efbb2 4889442430      mov     qword ptr [rsp+30h],rax
fffff803`334efbb7 488b442470      mov     rax,qword ptr [rsp+70h]
fffff803`334efbbc 4889442428      mov     qword ptr [rsp+28h],rax
fffff803`334efbc1 c744242044666c74 mov     dword ptr [rsp+20h],746C6644h
fffff803`334efbc9 e812000000      call    nt!ObpReferenceObjectByHandleWithTag (fffff803`334efbe0)
fffff803`334efbce 4883c448        add     rsp,48h
fffff803`334efbd2 c3              ret