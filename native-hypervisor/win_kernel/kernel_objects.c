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
    return GetObjectField(ethread, ETHREAD_KPROCESS, eprocess);
}

STATUS TranslateHandleToObject(IN HANDLE handle, OUT BYTE_PTR* object)
{

}

STATUS GetObjectField(IN BYTE_PTR object, IN QWORD field, OUT PVOID value)
{
    switch(field)
    {
        case ETHREAD_KPROCESS: 
        case EPROCESS_PID:
        case EPROCESS_OBJECT_TABLE:
            return CopyGuestMemory(value, object + field, sizeof(QWORD));
    }

    return STATUS_OBJECT_FIELD_NOT_FOUND;
}