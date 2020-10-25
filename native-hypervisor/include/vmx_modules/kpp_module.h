#ifndef __KPP_MODULE_H_
#define __KPP_MODULE_H_

#include <types.h>
#include <vmm/vmm.h>
#include <x86_64.h>

#define KPP_MODULE_MAX_COUNT 100

typedef struct _PATCH_GAURD_ENTRY
{
    QWORD address;
    // Max instruction length in x86 is 15 bytes
    BYTE instruction[X86_MAX_INSTRUCTION_LEN];
    BYTE instrutcionLength;
} PATCH_GAURD_ENTRY, *PPATCH_GAURD_ENTRY;


typedef struct _KPP_MODULE_DATA
{
    // First: Address, Second: Fake data
    PATCH_GAURD_ENTRY savedAddresses[KPP_MODULE_MAX_COUNT];
    QWORD savedAddressesCount;
} KPP_MODULE_DATA, *PKPP_MODULE_DATA;

STATUS KppHandleEptViolation(IN PCURRENT_GUEST_STATE data, IN PMODULE module);
STATUS RegisterNewProtectedKppEntry(IN QWORD guestPhysicalAddress, IN BYTE_PTR instruction, 
    IN BYTE instructionLength, IN PMODULE kppModule);
STATUS KppModuleInitialize(IN PSHARED_CPU_DATA sharedData, IN PMODULE module, 
    IN GENERIC_MODULE_DATA initData);
STATUS EmulatePatchGuardAction(IN PPATCH_GAURD_ENTRY kppEntries, IN QWORD count,
        IN QWORD address, IN BYTE instructionLength);
STATUS KppHandleEptViolation(IN PCURRENT_GUEST_STATE data, IN PMODULE module);


#endif