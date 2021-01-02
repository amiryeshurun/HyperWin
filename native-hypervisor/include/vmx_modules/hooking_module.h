#ifndef __HOOKING_MODULE_H_
#define __HOOKING_MODULE_H_

#include <types.h>
#include <vmm/vmm.h>
#include <vmx_modules/module.h>
#include <win_kernel/syscall_handlers.h>
#include <utils/map.h>
#include <utils/set.h>
#include <win_kernel/file.h>

// Used to determine how many vm-exits should occur before hooking the system calls
#define COUNT_UNTIL_HOOK 1500

// Used to determine if a physical address is assosiated with a return event or not
#define RETURN_EVENT_FLAG (1 << 30)

typedef STATUS (*HOOK_HANDLER)();

typedef struct _HOOK_CONTEXT
{
    HOOK_HANDLER handler;
    PVOID additionalData;
} HOOK_CONTEXT, *PHOOK_CONTEXT;

typedef struct _HOOKING_MODULE_EXTENSION
{
    PSYSCALL_DATA syscallsData;
    BOOL startExitCount;
    QWORD exitCount;
    BYTE_PTR ntoskrnl;
    BYTE_PTR win32k;
    BYTE_PTR lstar;
    QWORD guestCr3;
    QWORD_MAP addressToContext;
    QWORD_MAP filesData;
} HOOKING_MODULE_EXTENSION, *PHOOKING_MODULE_EXTENSION;

STATUS HookingModuleInitializeAllCores(IN PSHARED_CPU_DATA sharedData, IN PMODULE module, IN PGENERIC_MODULE_DATA initData);
STATUS HookingModuleInitializeSingleCore(IN PSINGLE_CPU_DATA data);
STATUS HookingDefaultHandler(IN PCURRENT_GUEST_STATE sharedData, IN PMODULE module);
STATUS HookingHandleMsrWrite(IN PCURRENT_GUEST_STATE data, IN PMODULE module);
STATUS HookingHandleException(IN PCURRENT_GUEST_STATE data, IN PMODULE module);

// Operational functions
STATUS HookingLocateSSDT(IN BYTE_PTR lstar, OUT BYTE_PTR* ssdt, IN QWORD guestCr3);
VOID HookingGetSystemTables(IN BYTE_PTR ssdt, OUT BYTE_PTR* ntoskrnl, OUT BYTE_PTR* win32k, IN QWORD guestCr3);
STATUS HookingHookSystemCalls(IN PMODULE module, IN QWORD guestCr3, IN BYTE_PTR ntoskrnl, IN BYTE_PTR win32k, 
    IN QWORD count, ...);


#endif