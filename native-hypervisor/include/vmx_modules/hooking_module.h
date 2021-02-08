#ifndef __HOOKING_MODULE_H_
#define __HOOKING_MODULE_H_

#include <types.h>
#include <vmm/vmm.h>
#include <vmx_modules/module.h>
#include <utils/map.h>
#include <utils/set.h>
#include <utils/list.h>

// Used to determine how many vm-exits should occur before hooking the system calls
#define COUNT_UNTIL_HOOK 1500

// The INT3 instruction used to detect return events of system call is (ADDRESS + 1)
#define CALC_RETURN_HOOK_ADDR(address) (address + 1)

// Define a name for the module to allow getting a module by its name
#define HOOKING_MODULE_NAME "HyperWin Hooking Module"

enum
{
    HOOK_TYPE_GENERIC,
    HOOK_TYPE_SYSCALL
};

typedef struct _SYSCALL_CONFIG_HOOK_CONTEXT
{
    QWORD syscallId;
} SYSCALL_CONFIG_HOOK_CONTEXT, *PSYSCALL_CONFIG_HOOK_CONTEXT;

typedef struct _CONFIG_HOOK_CONTEXT
{
    PCHAR name;
    BYTE type;
    BYTE offsetFromBeginning;
    BYTE instructionLength;
    BYTE params;
    QWORD guestPhysicalAddress;
    BYTE hookedInstruction[X86_MAX_INSTRUCTION_LEN];
    PVOID additionalData;
} CONFIG_HOOK_CONTEXT, *PCONFIG_HOOK_CONTEXT;

struct _HOOK_CONTEXT;

typedef STATUS (*HOOK_HANDLER)(struct _HOOK_CONTEXT*);

typedef struct _HOOK_CONTEXT
{
    HOOK_HANDLER handler;
    PCONFIG_HOOK_CONTEXT relatedConfig;
    QWORD virtualAddress;
    PVOID additionalData;
} HOOK_CONTEXT, *PHOOK_CONTEXT;

typedef struct _HOOKING_MODULE_EXTENSION
{
    BOOL startExitCount;
    QWORD exitCount;
    BYTE_PTR ntoskrnl;
    BYTE_PTR win32k;
    BYTE_PTR lstar;
    QWORD guestCr3;
    QWORD_MAP addressToContext;
    BYTE_PTR hookingConfigSegment;
    LIST hookConfig;
} HOOKING_MODULE_EXTENSION, *PHOOKING_MODULE_EXTENSION;

// VM-X related operations
STATUS HookingModuleInitializeAllCores(IN PMODULE module);
STATUS HookingModuleInitializeSingleCore(IN PMODULE module);
STATUS HookingDefaultHandler(IN PCURRENT_GUEST_STATE sharedData, IN PMODULE module);
STATUS HookingHandleMsrWrite(IN PCURRENT_GUEST_STATE data, IN PMODULE module);
STATUS HookingHandleException(IN PCURRENT_GUEST_STATE data, IN PMODULE module);

// Operational functions
STATUS HookingLocateSSDT(IN BYTE_PTR lstar, OUT BYTE_PTR* ssdt, IN QWORD guestCr3);
VOID HookingGetSystemTables(IN BYTE_PTR ssdt, OUT BYTE_PTR* ntoskrnl, OUT BYTE_PTR* win32k, IN QWORD guestCr3);
STATUS HookingHookSystemCalls(IN QWORD guestCr3, IN BYTE_PTR ntoskrnl, IN BYTE_PTR win32k, 
    IN QWORD count, ...);
STATUS HookingSetupGenericHook(IN QWORD guestVirtualAddress, IN PCHAR name, IN HOOK_HANDLER handler,
    IN HOOK_HANDLER returnHandler);
STATUS HookingParseConfig(IN BYTE_PTR hookConfigSegment, IN PLIST hookConfig);
STATUS HookingTranslateSyscallNameToId(IN PCHAR name, OUT QWORD_PTR syscallId);
STATUS HookingRemoveHook(IN PCHAR name);

extern PVOID __hooking_config_segment;

#endif