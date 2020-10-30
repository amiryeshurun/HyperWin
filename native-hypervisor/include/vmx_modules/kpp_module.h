#ifndef __KPP_MODULE_H_
#define __KPP_MODULE_H_

#include <types.h>
#include <vmm/vmm.h>
#include <x86_64.h>
#include <win_kernel/syscall_handlers.h>

#define KPP_MODULE_MAX_COUNT 100

typedef struct _KPP_MODULE_DATA
{
    PSYSCALL_DATA syscallsData;
    QWORD hookedSyscalls[KPP_MODULE_MAX_COUNT];
    QWORD hookedSyscallsCount;
} KPP_MODULE_DATA, *PKPP_MODULE_DATA;

STATUS KppHandleEptViolation(IN PCURRENT_GUEST_STATE data, IN PMODULE module);
STATUS RegisterNewProtectedKppEntry(IN QWORD syscallId, IN QWORD guestPhysicalAddress, IN BYTE_PTR instruction, 
    IN BYTE instructionLength, IN PMODULE kppModule);
STATUS KppModuleInitializeAllCores(IN PSHARED_CPU_DATA sharedData, IN PMODULE module, IN GENERIC_MODULE_DATA initData);
STATUS KppModuleInitializeSingleCore(IN PSINGLE_CPU_DATA data);
STATUS EmulatePatchGuardAction(IN PKPP_MODULE_DATA kppData, IN QWORD address, IN BYTE instructionLength);
STATUS KppHandleEptViolation(IN PCURRENT_GUEST_STATE data, IN PMODULE module);


#endif