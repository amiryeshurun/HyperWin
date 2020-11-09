#include <win_kernel/kernel_objects.h>
#include <win_kernel/memory_manager.h>

STATUS GetCurrent_ETHREAD(IN BYTE_PTR kprcb, OUT BYTE_PTR* ethread)
{
    return CopyGuestMemory(ethread, kprcb + KPRCB_KTHREAD, sizeof(QWORD));
}

STATUS GetCurrent_EPROCESS(IN BYTE_PTR kprcb, OUT BYTE_PTR* eprocess)
{
    BYTE_PTR ethread;
    if(GetCurrent_ETHREAD(kprcb, &ethread) != STATUS_SUCCESS)
        return STATUS_ETHREAD_NOT_AVAILABLE;
    Print("ETHREAD: %8\n", ethread);
    return GetObjectField(ethread, KAPC_STATE_KPROCESS, eprocess);
}

STATUS GetObjectField(IN BYTE_PTR object, IN QWORD field, OUT PVOID value)
{
    switch(field)
    {
        case KAPC_STATE_KPROCESS:
            return CopyGuestMemory(value, object + ETHREAD_KAPC_STATE + KAPC_STATE_KPROCESS, 
                sizeof(QWORD));
        case EPROCESS_PID:
            return CopyGuestMemory(value, object + EPROCESS_PID, sizeof(QWORD));
    }

    return STATUS_OBJECT_FIELD_NOT_FOUND;
}