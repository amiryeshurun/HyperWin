#include <utils/utils.h>
#include <bios/bios_os_loader.h>
#include <vmm/vmm.h>
#include <vmm/memory_manager.h>
#include <intrinsics.h>
#include <debug.h>
#include <x86_64.h>
#include <bios/apic.h>
#include <utils/allocation.h>
#include <vmm/vm_operations.h>
#include <vmm/vmcs.h>
#include <win_kernel/utils.h>

VOID BiosInitialize()
{
    VmmSetupVirtualAddress(__readcr3());
    BiosInitializeHypervisorsSharedData(CODE_BEGIN_ADDRESS, 0x000fffffULL);
    BiosLoadMBRToEntryPoint();
    HwCopyMemory((BYTE_PTR)REAL_MODE_CODE_START, (BYTE_PTR)SetupSystemAndHandleControlToBios,
        SetupSystemAndHandleControlToBiosEnd - SetupSystemAndHandleControlToBios);
}

VOID BiosInitializeHypervisorsSharedData(IN QWORD codeBase, IN QWORD codeLength)
{
    BYTE rsdtType, numberOfCores, processorIdentifires[MAX_CORES];
    BYTE_PTR apicTable, rsdtTable, physicalHypervisorBase;
    WORD memoryRegionsCount;
    QWORD allocationSize, hypervisorBase;
    DWORD validRamCount;
    PSHARED_CPU_DATA sharedData;

    ASSERT(BiosFindRSDT(&rsdtTable, &rsdtType) == STATUS_SUCCESS);
    ASSERT(BiosLocateSystemDescriptorTable(rsdtTable, &apicTable, rsdtType, "APIC") == STATUS_SUCCESS);
    ASSERT(ApicGetCoresData(apicTable, &numberOfCores, processorIdentifires) == STATUS_SUCCESS);
    BiosEnterRealModeRunFunction(GET_MEMORY_MAP, NULL);
    memoryRegionsCount = *((WORD_PTR)E820_OUTPUT_ADDRESS);
    PE820_LIST_ENTRY memoryMap = (PE820_LIST_ENTRY)(E820_OUTPUT_ADDRESS + 2);
    allocationSize = 0;
    allocationSize += ALIGN_UP(sizeof(SHARED_CPU_DATA), PAGE_SIZE);
    allocationSize += (ALIGN_UP(sizeof(SINGLE_CPU_DATA), PAGE_SIZE) * numberOfCores);
    allocationSize += (ALIGN_UP(sizeof(CURRENT_GUEST_STATE), PAGE_SIZE) * numberOfCores);
    if(BiosAllocateMemoryUsingMemoryMap(memoryMap, memoryRegionsCount, allocationSize, &physicalHypervisorBase))
    {
        Print("Allocation of %8 bytes failed.\n", allocationSize);
        ASSERT(FALSE);
    }
    hypervisorBase = PhysicalToVirtual(physicalHypervisorBase);
    HwSetMemory(hypervisorBase, 0, allocationSize);
    sharedData = hypervisorBase;
    sharedData->numberOfCores = numberOfCores;
    sharedData->hypervisorBase = hypervisorBase;
    sharedData->physicalHypervisorBase = physicalHypervisorBase;
    sharedData->hypervisorBaseSize = ALIGN_UP(allocationSize, LARGE_PAGE_SIZE);
    sharedData->codeBase = PhysicalToVirtual(codeBase);
    sharedData->physicalCodeBase = codeBase;
    sharedData->codeBaseSize = ALIGN_UP(codeLength, PAGE_SIZE);
    // Initialize cores data
    for(BYTE i = 0; i < numberOfCores; i++)
    {
        sharedData->cpuData[i] = hypervisorBase + ALIGN_UP(sizeof(SHARED_CPU_DATA), PAGE_SIZE) 
            + i * ALIGN_UP(sizeof(SINGLE_CPU_DATA), PAGE_SIZE);
        sharedData->currentState[i] = hypervisorBase + ALIGN_UP(sizeof(SHARED_CPU_DATA), PAGE_SIZE) 
            + numberOfCores * ALIGN_UP(sizeof(SINGLE_CPU_DATA), PAGE_SIZE)
            + i * ALIGN_UP(sizeof(CURRENT_GUEST_STATE), PAGE_SIZE);
        sharedData->currentState[i]->currentCPU = sharedData->cpuData[i];
        sharedData->cpuData[i]->sharedData = sharedData;
        sharedData->cpuData[i]->coreIdentifier = processorIdentifires[i];
    }
    validRamCount = 0;
    for(DWORD i = 0; i < memoryRegionsCount; i++)
    {
        if(memoryMap[i].type == E820_USABLE_REGION)
        {
            sharedData->validRam[i].baseAddress = memoryMap[i].baseAddress;
            sharedData->validRam[i].length = memoryMap[i].length;
            sharedData->validRam[i].type = E820_USABLE_REGION;
            sharedData->validRam[i].extendedAttribute = memoryMap[i].extendedAttribute;
            validRamCount++;
        }
        sharedData->allRam[i].baseAddress = memoryMap[i].baseAddress;
        sharedData->allRam[i].length = memoryMap[i].length;
        sharedData->allRam[i].type = memoryMap[i].type;
        sharedData->allRam[i].extendedAttribute = memoryMap[i].extendedAttribute;
    }
    sharedData->memoryRangesCount = memoryRegionsCount;
    sharedData->validRamCount = validRamCount;    
    // Init heap before registering modules
    HeapInit(&sharedData->heap, HEAP_SIZE, HEAP_FREE_CYCLE, HeapAllocate, HeapDeallocate, HeapDefragment);
    // Initialize hypervisor on BSP
    VmmInitializeSingleHypervisor(sharedData->cpuData[0]);
    // Register modules global data
    VmmGlobalRegisterAllModules();
    // Init modules on BSP
    VmmInitModulesSingleCore();
    // Init all components which are not a part of a vmx module
    WinInitializeComponents();
    // Back to VMX-non root mode
    if(VmmSetupCompleteBackToGuestState() != STATUS_SUCCESS)
    {
        // Should never arrive here
        Print("FLAGS: %8, instruction error: %8\n", __readflags(), vmread(VM_INSTRUCTION_ERROR));
        ASSERT(FALSE);
    }
    // Enable 2xAPIC
    ApicEnableX2APIC();
    // Initialize hypervisor on all cores except the BSP
    for(QWORD i = 1; i < numberOfCores; i++)
        ASSERT(ApicActivateHypervisorOnProcessor(processorIdentifires[i], sharedData->cpuData[i])
            == STATUS_SUCCESS);
    ASSERT(BiosHideCodeBase(sharedData->allRam, &sharedData->memoryRangesCount, codeBase, codeLength) == STATUS_SUCCESS);
    // Hook E820
    ASSERT(VmmSetupE820Hook(sharedData) == STATUS_SUCCESS);
}

STATUS BiosAllocateMemoryUsingMemoryMap
    (IN PE820_LIST_ENTRY memoryMap, IN DWORD memoryRegionsCount, IN QWORD allocationSize, OUT BYTE_PTR* address)
{
    QWORD alignedAllocationSize, unalignedCountBase, unalignedCountLength;
    INT upperIdx;

    alignedAllocationSize = ALIGN_UP(allocationSize, LARGE_PAGE_SIZE);
    upperIdx = NEG_INF; // high addresses are more rarely to be in use on the computer startup, use them
    for(QWORD i = 0; i < memoryRegionsCount; i++)
    {
        if(memoryMap[i].type != E820_USABLE_REGION)
            continue;
        if(memoryMap[i].length <= allocationSize)
            continue;
        if((INT)i > upperIdx)
            upperIdx = i;
    }
    if(upperIdx == NEG_INF)
        return STATUS_NO_MEM_AVAILABLE;
    unalignedCountBase = memoryMap[upperIdx].baseAddress % PAGE_SIZE;
    unalignedCountLength = memoryMap[upperIdx].length % PAGE_SIZE;
    memoryMap[upperIdx].length -= (alignedAllocationSize + unalignedCountBase + unalignedCountLength);
    *address = memoryMap[upperIdx].baseAddress + memoryMap[upperIdx].length;
    return STATUS_SUCCESS;
}

VOID PrintMemoryRanges(IN PE820_LIST_ENTRY start, IN QWORD count)
{
    for(QWORD i = 0; i < count; i++)
        Print("######### %d\n"
               "Base address: %8\n"
               "Length: %8\n"
               "Type: %d\n", i, start[i].baseAddress, start[i].length, start[i].type);
}

STATUS BiosHideCodeBase
    (IN PE820_LIST_ENTRY memoryMap, OUT WORD_PTR updatedCount, IN QWORD codeBegin, IN QWORD codeLength)
{
    QWORD codeRegionIdx, count;

    codeRegionIdx = NEG_INF;
    count = *updatedCount;
    for(QWORD i = 0; i < count; i++)
    {
        if(memoryMap[i].type == E820_USABLE_REGION && (codeBegin > memoryMap[i].baseAddress) 
            && ((codeBegin + codeLength) < memoryMap[i].baseAddress + memoryMap[i].length))
        {
            if(codeRegionIdx == NEG_INF)
            {
                codeRegionIdx = i;
                continue;
            }
            // In some cases, the memory map is unsorted. Keep searching...
            if((codeBegin - memoryMap[codeRegionIdx].baseAddress) > (codeBegin - memoryMap[i].baseAddress))
                codeRegionIdx = i;
        }
    }
    if(codeRegionIdx == NEG_INF)
        return STATUS_CODE_REGION_NOT_FOUND;
    PrintDebugLevelDebug("The memory range for hypervisor code was found. Base: %8, Length %d\n", 
        memoryMap[codeRegionIdx].baseAddress, memoryMap[codeRegionIdx].length);
    if((codeBegin + codeLength) > (memoryMap[codeRegionIdx].baseAddress +  memoryMap[codeRegionIdx].length))
        return STATUS_CODE_IN_DIFFERENT_REGIONS;
    E820_LIST_ENTRY first = {
        .baseAddress = memoryMap[codeRegionIdx].baseAddress,
        .length = codeBegin - memoryMap[codeRegionIdx].baseAddress - PAGE_SIZE,
        .type = E820_USABLE_REGION,
        .extendedAttribute = memoryMap[codeRegionIdx].extendedAttribute
        },
        second = {
        .baseAddress = codeBegin + ALIGN_UP(codeLength, PAGE_SIZE),
        .length = ALIGN_DOWN(memoryMap[codeRegionIdx].length - codeLength - first.length - PAGE_SIZE
            ,PAGE_SIZE),
        .type = E820_USABLE_REGION,
        .extendedAttribute = memoryMap[codeRegionIdx].extendedAttribute
        };
    for(QWORD i = count; i > codeRegionIdx; i--)
        memoryMap[i] = memoryMap[i - 1];
    memoryMap[codeRegionIdx] = first;
    memoryMap[codeRegionIdx + 1] = second;
    *updatedCount = count + 1;
    return STATUS_SUCCESS;
}