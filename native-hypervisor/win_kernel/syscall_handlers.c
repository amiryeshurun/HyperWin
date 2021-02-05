#include <win_kernel/syscall_handlers.h>
#include <win_kernel/memory_manager.h>
#include <debug.h>
#include <vmm/vm_operations.h>
#include <win_kernel/kernel_objects.h>
#include <vmx_modules/hooking_module.h>
#include <win_kernel/utils.h>
#include <win_kernel/irp.h>
#include <win_kernel/file.h>

STATUS ShdHandleNtReadFile(IN PHOOK_CONTEXT context)
{
    PCURRENT_GUEST_STATE state;
    PSHARED_CPU_DATA shared;
    PREGISTERS regs;
    QWORD params[17];
    QWORD fileObject, handleTable, eprocess, returnAddress, scb, fcb, fileIndex,
        vpb, deviceObj, driverObj, irpMjRead, fastIoDispatch, fastIoRead;
    WORD fileType;
    WIN_KERNEL_UNICODE_STRING driverName;
    STATUS status = STATUS_SUCCESS;

    state = VmmGetVmmStruct();
    shared = state->currentCPU->sharedData;
    regs = &state->guestRegisters;
    // Receive syscall parameters
    WinGetParameters(params, context->relatedConfig->params);
    // Translate the first parameter (file handle) to the corresponding _FILE_OBJECT structure
    ObjGetCurrent_EPROCESS(&eprocess);
    ObjGetObjectField(EPROCESS, eprocess, EPROCESS_OBJECT_TABLE, &handleTable);
    if(ObjTranslateHandleToObject(params[0], handleTable, &fileObject) != STATUS_SUCCESS)
    {
#ifdef DEBUG_HANDLE_TRANSLATION_FAILURE
        Print("Could not translate handle to object, skipping... (Handle value: %8)\n", params[0]);
#endif
        goto cleanup;
    }
    // Check if this is a file object (See MSDN file object page)
    ObjGetObjectField(FILE_OBJECT, fileObject, FILE_OBJECT_TYPE, &fileType);
    if(fileType != 5)
        goto cleanup;
    // Try to hook NtfsFsdRead instead of NtReadFile (for kernel mode data protection)
    ObjGetObjectField(FILE_OBJECT, fileObject, FILE_OBJECT_VPB, &vpb);
    if(vpb)
    {
        // Ntfs
        BYTE nameAsBytes[] = { 0x5C, 0x00, 0x46, 0x00, 0x69, 0x00, 0x6C, 0x00, 0x65, 0x00, 0x53, 0x00, 0x79, 0x00, 0x73, 0x00,
         0x74, 0x00, 0x65, 0x00, 0x6D, 0x00, 0x5C, 0x00, 0x4E, 0x00, 0x74, 0x00, 0x66, 0x00, 0x73, 0x00 };
        BYTE driverNameBuffer[50];
        // Check the name of the driver
        ObjGetObjectField(VPB, vpb, VPB_DEVICE_OBJECT, &deviceObj);
        ObjGetObjectField(DEVICE_OBJECT, deviceObj, DEVICE_OBJECT_DRIVER_OBJECT, &driverObj);
        ObjGetObjectField(DRIVER_OBJECT, driverObj, DRIVER_OBJECT_NAME, &driverName);
        WinMmCopyGuestMemory(driverNameBuffer, driverName.address, driverName.length);
        // Is it Ntfs?
        if(!HwCompareMemory(driverNameBuffer, nameAsBytes, 32))
        {
            WinMmCopyGuestMemory(&irpMjRead, driverObj + DRIVER_OBJECT_MAJOR_FUNCTION + 
                sizeof(QWORD) * IRP_MJ_READ, sizeof(QWORD));
            Print("Found NtfsFsdRead at: %8, hooking now!\n", irpMjRead);
            ASSERT(HookingSetupGenericHook(irpMjRead, "NtfsFsdRead", FileHandleRead,
                FileHandleReadReturn) == STATUS_SUCCESS);
            ObjGetObjectField(DRIVER_OBJECT, driverObj, DRIVER_OBJECT_FAST_IO_DISPATCH, &fastIoDispatch);
            ObjGetObjectField(FAST_IO_DISPATCH, fastIoDispatch, FAST_IO_DISPATCH_FAST_IO_READ, &fastIoRead);
            Print("Found NtfsCopyReadA at: %8, hooking now!\n", fastIoRead);
            ASSERT(HookingSetupGenericHook(fastIoRead, "NtfsCopyReadA", FileHandleFastRead,
                FileHandleFastReadReturn) == STATUS_SUCCESS);
            Print("Removing NtReadFile...\n");
            ASSERT(HookingRemoveHook("NtReadFile") == STATUS_SUCCESS);
        }
    }
cleanup:
    // Emulate replaced instruction: mov rax,rsp
    regs->rax = regs->rsp;
    regs->rip += context->relatedConfig->instructionLength;
    // End emulation
    return status;
}
