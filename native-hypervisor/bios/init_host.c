#include <utils/utils.h>
#include <bios/bios_os_loader.h>
#include <vmm/vmm.h>
#include <vmm/memory_manager.h>
#include <intrinsics.h>
#include <debug.h>
#include <x86_64.h>
#include <bios/apic.h>
#include <guest_communication/communication_block.h>
#include <utils/allocation.h>

VOID Initialize()
{
    SetupVirtualAddress(__readcr3());
    InitializeHypervisorsSharedData(CODE_BEGIN_ADDRESS, 0x000ffffULL);
    LoadMBRToEntryPoint();
    CopyMemory((BYTE_PTR)REAL_MODE_CODE_START, (BYTE_PTR)SetupSystemAndHandleControlToBios,
        SetupSystemAndHandleControlToBiosEnd - SetupSystemAndHandleControlToBios);
}

VOID InitializeHypervisorsSharedData(IN QWORD codeBase, IN QWORD codeLength)
{
    BYTE rsdtType, numberOfCores, processorIdentifires[MAX_CORES];
    BYTE_PTR apicTable, rsdtTable;
    ASSERT(FindRSDT(&rsdtTable, &rsdtType) == STATUS_SUCCESS);
    ASSERT(LocateSystemDescriptorTable(rsdtTable, &apicTable, rsdtType, "APIC") == STATUS_SUCCESS);
    ASSERT(GetCoresData(apicTable, &numberOfCores, processorIdentifires) == STATUS_SUCCESS);
    EnterRealModeRunFunction(GET_MEMORY_MAP, NULL);
    WORD memoryRegionsCount = *((WORD_PTR)E820_OUTPUT_ADDRESS);
    PE820_LIST_ENTRY memoryMap = (PE820_LIST_ENTRY)(E820_OUTPUT_ADDRESS + 2);
    QWORD allocationSize = 0;
    allocationSize += ALIGN_UP(sizeof(SHARED_CPU_DATA), PAGE_SIZE);
    allocationSize += (ALIGN_UP(sizeof(SINGLE_CPU_DATA), PAGE_SIZE) * numberOfCores);
    allocationSize += (ALIGN_UP(sizeof(CURRENT_GUEST_STATE), PAGE_SIZE) * numberOfCores);
    BYTE_PTR physicalHypervisorBase;
    if(AllocateMemoryUsingMemoryMap(memoryMap, memoryRegionsCount, allocationSize, &physicalHypervisorBase))
    {
        Print("Allocation of %8 bytes failed.\n", allocationSize);
        ASSERT(FALSE);
    }
    BYTE_PTR physicalReadPipe;
    if(AllocateMemoryUsingMemoryMap(memoryMap, memoryRegionsCount, LARGE_PAGE_SIZE, &physicalReadPipe))
    {
        Print("Allocation of %8 bytes failed.\n", LARGE_PAGE_SIZE);
        ASSERT(FALSE);
    }
    BYTE_PTR physicalWritePipe;
    if(AllocateMemoryUsingMemoryMap(memoryMap, memoryRegionsCount, LARGE_PAGE_SIZE, &physicalWritePipe))
    {
        Print("Allocation of %8 bytes failed.\n", LARGE_PAGE_SIZE);
        ASSERT(FALSE);
    }
    ASSERT(HideCodeBase(memoryMap, &memoryRegionsCount, codeBase, codeLength) == STATUS_SUCCESS);
    QWORD hypervisorBase = PhysicalToVirtual(physicalHypervisorBase);
    SetMemory(hypervisorBase, 0, allocationSize);
    PSHARED_CPU_DATA sharedData = hypervisorBase;
    sharedData->numberOfCores = numberOfCores;
    sharedData->hypervisorBase = hypervisorBase;
    sharedData->physicalHypervisorBase = physicalHypervisorBase;
    sharedData->hypervisorBaseSize = ALIGN_UP(allocationSize, LARGE_PAGE_SIZE);
    sharedData->codeBase = PhysicalToVirtual(codeBase);
    sharedData->physicalCodeBase = codeBase;
    sharedData->codeBaseSize = ALIGN_UP(codeLength, PAGE_SIZE);
    // Save communication block area in global memory & initialize
    SetMemory(PhysicalToVirtual(physicalReadPipe), 0, LARGE_PAGE_SIZE);
    InitPipe(&(sharedData->readPipe), physicalReadPipe, PhysicalToVirtual(physicalReadPipe), 0);
    SetMemory(PhysicalToVirtual(physicalWritePipe), 0 , LARGE_PAGE_SIZE);
    InitPipe(&(sharedData->writePipe), physicalWritePipe, PhysicalToVirtual(physicalWritePipe), 0);
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
    DWORD validRamCount = 0;
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
    sharedData->wereModulesInitiated = FALSE;
    // Init heap before registering modules
    HeapInit(&(sharedData->heap), HEAP_SIZE, HEAP_FREE_CYCLE, HeapAllocate, HeapDeallocate, HeapDefragment);
    // Enable 2xAPIC
    EnableX2APIC();
    // Initialize hypervisor on all cores except the BSP
    for(QWORD i = 1; i < numberOfCores; i++)
        ASSERT(ActivateHypervisorOnProcessor(processorIdentifires[i], sharedData->cpuData[i])
            == STATUS_SUCCESS);
    // Initialize hypervisor on BSP
    InitializeSingleHypervisor(sharedData->cpuData[0]);
    // Hook E820
    ASSERT(SetupE820Hook(sharedData) == STATUS_SUCCESS);
}

STATUS AllocateMemoryUsingMemoryMap
    (IN PE820_LIST_ENTRY memoryMap, IN DWORD memoryRegionsCount, IN QWORD allocationSize, OUT BYTE_PTR* address)
{
    QWORD alignedAllocationSize = ALIGN_UP(allocationSize, LARGE_PAGE_SIZE);
    INT upperIdx = NEG_INF; // high addresses are more rarely to be in use on the computer startup, use them
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
    QWORD unalignedCountBase = memoryMap[upperIdx].baseAddress % PAGE_SIZE;
    QWORD unalignedCountLength = memoryMap[upperIdx].length % PAGE_SIZE;
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

STATUS HideCodeBase
    (IN PE820_LIST_ENTRY memoryMap, OUT WORD_PTR updatedCount, IN QWORD codeBegin, IN QWORD codeLength)
{
    QWORD codeRegionIdx = NEG_INF, count = *updatedCount;
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