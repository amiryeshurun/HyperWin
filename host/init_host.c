#include <types.h>
#include <utils.h>
#include <guest/bios_os_loader.h>
#include <host/vmm.h>

extern VOID UpdateInstructionPointer(QWORD offset);
extern VOID SetupSystemAndHandleControlToBios();
extern VOID SetupSystemAndHandleControlToBiosEnd();

VOID Initialize()
{
    // InitializeHypervisorsSharedData(CODE_BEGIN_ADDRESS, 0x000fffffULL);
    LoadMBRToEntryPoint();
    CopyMemory((QWORD_PTR)REAL_MODE_CODE_START, (QWORD_PTR)SetupSystemAndHandleControlToBios,
        SetupSystemAndHandleControlToBiosEnd - SetupSystemAndHandleControlToBios);
}

VOID InitializeHypervisorsSharedData(IN QWORD codeBase, IN QWORD codeLength)
{
    BYTE numberOfCores; // TBD...
    EnterRealModeRunFunction(GET_MEMORY_MAP, NULL);
    WORD memoryRegionsCount = *((WORD_PTR)E820_OUTPUT_ADDRESS);
    PE820_LIST_ENTRY memoryMap = (PE820_LIST_ENTRY)(E820_OUTPUT_ADDRESS + 2);
    QWORD allocationSize = 0;
    allocationSize += ALIGN_UP(codeBase, PAGE_SIZE);
    allocationSize += ALIGN_UP(sizeof(SHARED_CPU_DATA), PAGE_SIZE);
    allocationSize += (ALIGN_UP(sizeof(SINGLE_CPU_DATA), PAGE_SIZE) * numberOfCores);
    allocationSize += (ALIGN_UP(sizeof(CURRENT_GUEST_STATE) * numberOfCores, PAGE_SIZE));
    QWORD physicalHypervisorBase = AllocateMemoryUsingMemoryMap(memoryMap, memoryRegionsCount, allocationSize);
    QWORD hypervisorBase = PhysicalToVirtual(physicalHypervisorBase);
    /// TODO: ZERO THE MEMORY
    CopyMemory(hypervisorBase, codeBase, codeBase);
    UpdateInstructionPointer(hypervisorBase - codeBase);
    PSHARED_CPU_DATA sharedData = hypervisorBase + ALIGN_UP(codeBase, PAGE_SIZE);
    for(BYTE i = 0; i < numberOfCores; i++)
    {
        sharedData->cpuData[i] = hypervisorBase + ALIGN_UP(codeBase, PAGE_SIZE) 
            + ALIGN_UP(sizeof(SHARED_CPU_DATA), PAGE_SIZE) + i * ALIGN_UP(sizeof(SINGLE_CPU_DATA), PAGE_SIZE);
        sharedData->currentState[i] = hypervisorBase + ALIGN_UP(codeBase, PAGE_SIZE) 
            + ALIGN_UP(sizeof(SHARED_CPU_DATA), PAGE_SIZE) + numberOfCores * ALIGN_UP(sizeof(SINGLE_CPU_DATA), PAGE_SIZE)
            + i * sizeof(CURRENT_GUEST_STATE);
        sharedData->currentState[i] = sharedData->cpuData[i];
        sharedData->cpuData[i]->sharedData = sharedData;
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
}

QWORD AllocateMemoryUsingMemoryMap
    (IN PE820_LIST_ENTRY memoryMap, IN DWORD memoryRegionsCount, IN QWORD allocationSize)
{
    QWORD alignedAllocationSize = ALIGN_UP(allocationSize, LARGE_PAGE_SIZE);
    int upperIdx = NEG_INF; // high addresses are more rarely to be in use on the computer startup, use them
    for(DWORD i = 0; i < memoryRegionsCount; i++)
    {
        if(memoryMap[i].type != E820_USABLE_REGION)
            continue;
        if(memoryMap[i].length <= allocationSize)
            continue;
        if(i > upperIdx)
            upperIdx = i;
    }
    if(upperIdx == NEG_INF)
        return NO_MEM_AVAILABLE;
    memoryMap[upperIdx].length -= alignedAllocationSize;
    return memoryMap[upperIdx].baseAddress + memoryMap[upperIdx].length;
}