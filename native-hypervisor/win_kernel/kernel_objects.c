#include <win_kernel/kernel_objects.h>
#include <win_kernel/memory_manager.h>
#include <debug.h>
#include <vmm/vm_operations.h>
#include <vmm/vmcs.h>

BYTE_PTR GetCurrent_KPRCB()
{
    return vmread(GUEST_GS_BASE) + KPRC_KPRCB;
}

STATUS GetCurrent_ETHREAD(OUT BYTE_PTR* ethread)
{
    return CopyGuestMemory(ethread, GetCurrent_KPRCB() + KPRCB_KTHREAD, sizeof(QWORD));
}

STATUS GetCurrent_EPROCESS(OUT BYTE_PTR* eprocess)
{
    BYTE_PTR ethread;
    if(GetCurrent_ETHREAD(&ethread) != STATUS_SUCCESS)
        return STATUS_ETHREAD_NOT_AVAILABLE;
    return GetObjectField(ETHREAD, ethread, ETHREAD_KPROCESS, eprocess);
}

/* 
    This function is based on the assembly code of ExpLookupHandleTableEntry.
    To have better a understanding, see ReactOS' implementation before reading this.
*/
STATUS TranslateHandleToObject(IN HANDLE handle, IN BYTE_PTR handleTable, OUT BYTE_PTR* object)
{
    // Zero the 2 last bits (tag bits)
    handle &= ~(3ULL);
    DWORD nextHandleNeedingPool;
    if(CopyGuestMemory(&nextHandleNeedingPool, handleTable, sizeof(DWORD)) != STATUS_SUCCESS)
        return STATUS_COULD_NOT_TRANSLATE_HANDLE;
    if(handle >= nextHandleNeedingPool)
        return STATUS_COULD_NOT_TRANSLATE_HANDLE;
    QWORD tableBase;
    if(CopyGuestMemory(&tableBase, handleTable + 0x8, sizeof(QWORD)) != STATUS_SUCCESS)
        return STATUS_COULD_NOT_TRANSLATE_HANDLE;
    QWORD tableLevel = tableBase & 3, tableResult;
    // mov rax,rdx (case 1) OR mov rcx,rdx (case 2)
    HANDLE handleBackup = handle;
    switch(tableLevel)
    {
        case 2:
        {    
            // rcx = handleBackup, rdx = handle, rax = handleBackup >> 0xa (res)
            // shr rcx,0Ah
            handleBackup >>= 0xa;
            // mov rax,rcx
            QWORD res = handleBackup;
            // and ecx,1FFh
            handleBackup &= 0x1ff;
            // shr rax,9
            res >>= 9;
            // and edx,3FFh
            handle &= 0x3ff;
            // mov rax,qword ptr [r8+rax*8-2]
            if(CopyGuestMemory(&res, tableBase + res * 8 - 2, sizeof(QWORD)) != STATUS_SUCCESS)
                return STATUS_COULD_NOT_TRANSLATE_HANDLE;
            // mov rax,qword ptr [rax+rcx*8]
            if(CopyGuestMemory(&res, res + handleBackup * 8, sizeof(QWORD)) != STATUS_SUCCESS)
                return STATUS_COULD_NOT_TRANSLATE_HANDLE;
            // lea rax,[rax+rdx*4]
            tableResult = res + handle * 4;
            break;
        }
        case 1:
        {
            // shr rax,0Ah
            handleBackup >>= 0xa;
            // and edx,3FFh
            handle &= 0x3ff;
            // mov rax,qword ptr [r8+rax*8-1]
            if(CopyGuestMemory(&handleBackup, tableBase + handleBackup * 8 - 1, sizeof(QWORD)) != STATUS_SUCCESS)
                return STATUS_COULD_NOT_TRANSLATE_HANDLE;
            // lea rax,[rax+rdx*4]
            tableResult = handleBackup + handle * 4;
            break;             
        }
        case 0:
        {
            // lea rax,[r8+rdx*4]
            tableResult = tableBase + handle * 4;
            break;
        }
        default:
        {
            Print("Wrong table level when translating handle to kernel object");
            ASSERT(FALSE);
        }
    }
    QWORD objectHeader;
    if(CopyGuestMemory(&objectHeader, tableResult, sizeof(QWORD)))
        return STATUS_COULD_NOT_TRANSLATE_HANDLE;
    objectHeader >>= 20;
    objectHeader *= 0x10;
    objectHeader += 0xffff000000000000ULL;
    *object = objectHeader + OBJECT_HEADER_BODY;
    return STATUS_SUCCESS;
}

STATUS Get_ETHREAD_field(IN IN BYTE_PTR object, IN QWORD field, OUT PVOID value)
{
    switch(field)
    {
        case ETHREAD_KPROCESS:
            return CopyGuestMemory(value, object + field, sizeof(QWORD));
    }
}
STATUS Get_EPROCESS_field(IN BYTE_PTR object, IN QWORD field, OUT PVOID value)
{
    switch(field)
    {
        case EPROCESS_PID:
        case EPROCESS_OBJECT_TABLE:
            return CopyGuestMemory(value, object + field, sizeof(QWORD));
    }
}

STATUS GetObjectField(IN BYTE objectType, IN BYTE_PTR object, IN QWORD field, OUT PVOID value)
{
    switch(objectType)
    {
        case ETHREAD:
            return Get_ETHREAD_field(object, field, value);
        case EPROCESS:
            return Get_EPROCESS_field(object, field, value);
        default:
        {
            Print("Unsupported object type!\n");
            ASSERT(FALSE);
        }
    }

    return STATUS_OBJECT_FIELD_NOT_FOUND;
}