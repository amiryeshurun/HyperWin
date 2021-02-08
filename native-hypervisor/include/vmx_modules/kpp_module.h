#ifndef __KPP_MODULE_H_
#define __KPP_MODULE_H_

#include <types.h>
#include <vmm/vmm.h>
#include <x86_64.h>
#include <win_kernel/syscall_handlers.h>
#include <utils/set.h>
#include <utils/list.h>

#define KPP_MODULE_MAX_COUNT 100
#define KPP_MODULE_NAME "KPP Module"

typedef struct _KPP_ENTRY_CONTEXT
{
    QWORD hookedInstructionLength;
    QWORD hookedInstructionAddress;
    BYTE hookedInstrucion[X86_MAX_INSTRUCTION_LEN];
} KPP_ENTRY_CONTEXT, *PKPP_ENTRY_CONTEXT;

typedef struct _KPP_MODULE_DATA
{
    QWORD_SET addressSet;
    LIST entriesList;
} KPP_MODULE_DATA, *PKPP_MODULE_DATA;

STATUS KppHandleEptViolation(IN PCURRENT_GUEST_STATE data, IN PMODULE module);
STATUS KppModuleInitializeAllCores(IN PMODULE module);
STATUS KppModuleInitializeSingleCore(IN PMODULE module);
STATUS KppEmulatePatchGuardAction(IN PKPP_MODULE_DATA kppData, IN QWORD address, IN BYTE instructionLength);
STATUS KppHandleEptViolation(IN PCURRENT_GUEST_STATE data, IN PMODULE module);
STATUS KppAddNewEntry(IN QWORD guestPhysicalAddress, IN QWORD hookedInstructionLength, IN BYTE_PTR hookedInstrucion);
STATUS KppRemoveEntry(IN QWORD guestPhysicalAddress);

#endif