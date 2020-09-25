#include <vmm/memory_manager.h>

VOID SetupVirtualAddress(IN QWORD pml4BaseAddress)
{
    ((QWORD_PTR)pml4BaseAddress)[200] = ((QWORD_PTR)pml4BaseAddress)[0];
}

QWORD VirtualToPhysical(IN QWORD address)
{
    return address - VIRTUAL_ADDRESS_OFFET;
}

QWORD PhysicalToVirtual(IN QWORD address)
{
    return address | VIRTUAL_ADDRESS_OFFET;
}