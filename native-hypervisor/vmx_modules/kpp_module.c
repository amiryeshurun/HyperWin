#include <vmx_modules/module.h>
#include <vmx_modules/kpp_module.h>
#include <vmm/vm_operations.h>
#include <vmm/vmcs.h>
#include <vmm/msr.h>
#include <win_kernel/memory_manager.h>
#include <debug.h>

STATUS KppModuleInitializeAllCores(IN PSHARED_CPU_DATA sharedData, IN PMODULE module, IN GENERIC_MODULE_DATA initData)
{
    PrintDebugLevelDebug("Starting initialization of KPP module for all cores\n");
    sharedData->heap.allocate(&sharedData->heap, sizeof(KPP_MODULE_DATA), &module->moduleExtension);
    SetMemory(module->moduleExtension, 0, sizeof(KPP_MODULE_DATA));
    PrintDebugLevelDebug("Shared cores data successfully initialized for KPP module\n");
    return STATUS_SUCCESS;
}

STATUS KppModuleInitializeSingleCore(IN PSINGLE_CPU_DATA data)
{
    PrintDebugLevelDebug("Starting initialization of KPP module on core #%d\n", data->coreIdentifier);
    PrintDebugLevelDebug("Finished initialization of KPP module on core #%d\n", data->coreIdentifier);
    return STATUS_SUCCESS;
}

STATUS RegisterNewProtectedKppEntry(IN QWORD guestPhysicalAddress, IN BYTE_PTR instruction, 
    IN BYTE instructionLength, IN PMODULE kppModule)
{
    PKPP_MODULE_DATA kppData = (PKPP_MODULE_DATA)kppModule->moduleExtension;
    if(kppData->savedAddresses >= KPP_MODULE_MAX_COUNT)
        return STATUS_NO_SPACE_AVAILABLE;
    kppData->savedAddresses[kppData->savedAddressesCount].address = guestPhysicalAddress;
    CopyMemory(kppData->savedAddresses[kppData->savedAddressesCount].instruction, instruction, 
        instructionLength);
    kppData->savedAddressesCount++;
    return STATUS_SUCCESS;
}

STATUS EmulatePatchGuardAction(IN PPATCH_GAURD_ENTRY kppEntries, IN QWORD count,
        IN QWORD address, IN BYTE instructionLength)
{
    BYTE_PTR hostVirtual = TranslateGuestPhysicalToHostVirtual(address);
    BYTE instruction[X86_MAX_INSTRUCTION_LEN];
    CopyMemory(instruction, hostVirtual, instructionLength);
    if(FALSE)
    {
        // still didn't search for KPP instruction
    }
    else
    {
        Print("New KPP instruction found: %.b\n", instructionLength, instruction);
        return STATUS_UNKNOWN_KPP_INSTRUCTION;
    }
    return STATUS_SUCCESS;
}

STATUS KppHandleEptViolation(IN PCURRENT_GUEST_STATE data, IN PMODULE module)
{
    QWORD address = vmread(GUEST_PHYSICAL_ADDRESS), instructionLength = vmread(VM_EXIT_INSTRUCTION_LEN);
    PKPP_MODULE_DATA kppData = (PKPP_MODULE_DATA)module->moduleExtension;
    return EmulatePatchGuardAction(kppData->savedAddresses, kppData->savedAddressesCount,
        address, instructionLength);
}