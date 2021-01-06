#include <win_kernel/process.h>
#include <win_kernel/memory_manager.h>
#include <win_kernel/kernel_objects.h>
#include <debug.h>

STATUS PspMarkProcessProtected(IN HANDLE processHandle, IN BYTE protectionLevel, IN BYTE signLevel, 
    IN BYTE sectionSignLevel)
{
    QWORD pid, eprocess, handleTable, protectedProcessEprocess;
    STATUS status;

    ObjGetCurrent_EPROCESS(&eprocess);
    ObjGetObjectField(EPROCESS, eprocess, EPROCESS_OBJECT_TABLE, &handleTable);
    if((status = ObjTranslateHandleToObject(processHandle, handleTable, &protectedProcessEprocess))
        != STATUS_SUCCESS)
    {
        Print("Could not translate handle to EPROCESS\n");
        return status;   
    }
    if((status = WinMmCopyMemoryToGuest(protectedProcessEprocess + EPROCESS_SIGN_LEVEL, &signLevel, sizeof(BYTE)))
         != STATUS_SUCCESS)
    {
        Print("Could not mark process as protected!\n");
        return status;
    }
    if((status = WinMmCopyMemoryToGuest(protectedProcessEprocess + EPROCESS_SECTION_SIGN_LEVEL, &sectionSignLevel, sizeof(BYTE))) 
        != STATUS_SUCCESS)
    {
        Print("Could not mark process as protected!\n");
        return status;
    }
    if((status = WinMmCopyMemoryToGuest(protectedProcessEprocess + EPROCESS_PROTECTION, &protectionLevel, sizeof(BYTE)))
        != STATUS_SUCCESS)
    {
        Print("Could not mark process as protected!\n");
        return status;
    }
    ObjGetObjectField(EPROCESS, protectedProcessEprocess, EPROCESS_PID, &pid);
    Print("Successfully marked process %d as protected with a protection level of: %d\n", pid, protectionLevel);
    return STATUS_SUCCESS;
}