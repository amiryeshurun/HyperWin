#ifndef __VMM_H_
#define __VMM_H_

#include <types.h>
#include <bios/bios_os_loader.h>
#include <x86_64.h>
#include <utils/allocation.h>
#include <vmx_modules/module.h>

/* Paging related data */
#define ARRAY_PAGE_SIZE (PAGE_SIZE / 8)
#define COMPUTER_MEM_SIZE 16
#define STACK_SIZE (4 * PAGE_SIZE)

/* CPU related data */
#define MAX_CORES 8

/* VMCS related data */
#define VMCS_SELECTOR_UNUSABLE (1 << 16)

/* HOST SELECTORS */
#define HYPERVISOR_CS_SELECTOR 8
#define HYPERVISOR_DS_SELECTOR 16

/* VMCALL agreed values */
#define VMCALL_SETUP_BASE_PROTECTION 0x11223344

struct _SINGLE_CPU_DATA;
struct _CURRENT_GUEST_STATE;

typedef struct _SHARED_CPU_DATA
{
    HEAP heap;
    MODULE defaultModule;
    PMODULE* modules;
    QWORD modulesCount;
    struct _SINGLE_CPU_DATA* cpuData[MAX_CORES];
    struct _CURRENT_GUEST_STATE* currentState[MAX_CORES];
    E820_LIST_ENTRY validRam[E820_OUTPUT_MAX_ENTRIES];
    WORD validRamCount;
    E820_LIST_ENTRY allRam[E820_OUTPUT_MAX_ENTRIES];
    WORD memoryRangesCount;
    BYTE numberOfCores;
    BYTE_PTR hypervisorBase;
    QWORD physicalHypervisorBase;
    QWORD hypervisorBaseSize;
    BYTE_PTR codeBase;
    QWORD physicalCodeBase;
    QWORD codeBaseSize;
    QWORD int15Segment;
    QWORD int15Offset;
} SHARED_CPU_DATA, *PSHARED_CPU_DATA;

typedef struct _SINGLE_CPU_DATA
{
    BYTE vmcs[PAGE_SIZE];
    BYTE vmxon[PAGE_SIZE];
    BYTE msrBitmaps[PAGE_SIZE];
    BYTE stack[STACK_SIZE];
    QWORD pageMapLevel4s[ARRAY_PAGE_SIZE]; // cr3
    QWORD pageDirectoryPointerTables[ARRAY_PAGE_SIZE]; // 1 PML entry = 512GB, enough for the HV
    QWORD pageDirectories[ARRAY_PAGE_SIZE * ARRAY_PAGE_SIZE]; // HV will use 2MB pages
    QWORD eptPageMapLevel4s[ARRAY_PAGE_SIZE];
    QWORD eptPageDirectoryPointerTables[ARRAY_PAGE_SIZE];
    /* Each 512 entries here are referenced by an entry from the previous table. 
        Each entry in the previous table maps 1GB of memory */
    QWORD eptPageDirectories[ARRAY_PAGE_SIZE * COMPUTER_MEM_SIZE]; 
    QWORD eptPageTables[ARRAY_PAGE_SIZE * ARRAY_PAGE_SIZE * COMPUTER_MEM_SIZE];
    BYTE coreIdentifier;
    QWORD gdt[0xff];
    MODULE defaultModule;
    PSHARED_CPU_DATA sharedData;
} SINGLE_CPU_DATA, *PSINGLE_CPU_DATA;

typedef struct _CURRENT_GUEST_STATE
{
    REGISTERS guestRegisters;
    PSINGLE_CPU_DATA currentCPU;
} CURRENT_GUEST_STATE, *PCURRENT_GUEST_STATE;

extern VOID VmmToVm();
extern VOID VmmHandleVmExit();
extern QWORD VmmSetupCompleteBackToGuestState();

VOID BiosInitializeHypervisorsSharedData(IN QWORD codeBase, IN QWORD codeLength);
VOID VmmInitializeSingleHypervisor(IN PVOID data);
DWORD VmmAdjustControls(IN DWORD control, IN QWORD msr);
VOID VmmHandleVmExitEx();
PCURRENT_GUEST_STATE VmmGetVmmStruct();
STATUS VmmSetupHypervisorCodeProtection(IN PSHARED_CPU_DATA data, IN QWORD codeBase, IN QWORD codeLength);
STATUS VmmUpdateEptAccessPolicy(IN PSINGLE_CPU_DATA data, IN QWORD base, IN QWORD length, IN QWORD access);
BOOL VmmCheckAccessToHiddenBase(IN PSHARED_CPU_DATA data, IN QWORD accessedAddress);
STATUS VmmSetupE820Hook(IN PSHARED_CPU_DATA sharedData);
VOID VmmGlobalRegisterAllModules();
VOID VmmInitModulesSingleCore();
STATUS VmmUpdateMsrAccessPolicy(IN PSINGLE_CPU_DATA data, IN QWORD msrNumber, IN BOOL read, IN BOOL write);
STATUS VmmInjectGuestInterrupt(IN BYTE vector, IN QWORD errorCode);

extern BYTE_PTR __modules_config_segment;
extern BYTE_PTR __modules_config_segment_end;

#endif