#include <types.h>
#include <utils.h>
#include <guest/bios_os_loader.h>
#include <host/vmm.h>
#include <intrinsics.h>
#include <debug.h>

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
    BYTE_PTR physicalHypervisorBase;
    if(AllocateMemoryUsingMemoryMap(memoryMap, memoryRegionsCount, allocationSize, &physicalHypervisorBase))
    {
        Print("Allocation of %d bytes failed.\n", allocationSize);
        ASSERT(FALSE);
    }
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

STATUS AllocateMemoryUsingMemoryMap
    (IN PE820_LIST_ENTRY memoryMap, IN DWORD memoryRegionsCount, IN QWORD allocationSize, OUT BYTE_PTR* address)
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
        return  STATUS_NO_MEM_AVAILABLE;
    memoryMap[upperIdx].length -= alignedAllocationSize;
    *address = memoryMap[upperIdx].baseAddress + memoryMap[upperIdx].length;
    return STATUS_SUCCESS;
}

STATUS FindRSDT(OUT BYTE_PTR* address, OUT QWORD_PTR type)
{
    CHAR pattern[] = "RSD PTR ";
    BYTE_PTR ebdaAddress = (*(WORD_PTR)EBDA_POINTER_ADDRESS) >> 4;
    BYTE_PTR rsdpBaseAddress;

    for(QWORD i = 0; i < 0x1024; i += 16)
    {
        if(!CompareMemory(ebdaAddress + i, pattern, 9))
        {
            rsdpBaseAddress = (ebdaAddress + i);
            goto RSDPFound;
        }
    }

    for(QWORD start = 0xE0000; start < 0xFFFFF; start += 16)
    {
        if(!CompareMemory(start, pattern, 9))
        {
            rsdpBaseAddress = start;
            goto RSDPFound;
        }
    }

    return STATUS_FAILURE;

RSDPFound:
    QWORD sum = 0;
    for(BYTE i = 0; i < RSDP_STRUCTURE_SIZE; i++)
            sum += rsdpBaseAddress[i];
    if(sum & 0xff) // checksum failed
        return STATUS_RSDP_INVALID_CHECKSUM;
    
    if(rsdpBaseAddress[RSDP_REVISION_OFFSET] == 1) // ACPI Vesrion 1.0
    {   
        *type = 1;
        *address = rsdpBaseAddress + RSDP_ADDRESS_OFFSET;
        return STATUS_SUCCESS;
    }
    // ACPI Vesrion 2.0 and above
    for(BYTE i = 0; i < RSDP_EXTENSION_SIZE; i++)
        sum += (rsdpBaseAddress + RSDP_STRUCTURE_SIZE)[i];
    if(sum & 0xff)
        return STATUS_RSDP_INVALID_CHECKSUM;
    *type = 2;
    *address = rsdpBaseAddress + RSDP_STRUCTURE_SIZE +  4; // Xsdt
    return STATUS_SUCCESS;
}

STATUS FindAPICTable(IN BYTE_PTR rsdt, OUT BYTE_PTR* apicTable, IN QWORD type)
{
    QWORD sum = 0;
    for(QWORD i = 0; i < *(DWORD_PTR)(rsdt + RSDT_LENGTH_OFFSET); i++)
        sum += rsdt[i];
    if(sum % 0x100)
        return STATUS_RSDT_INVALID_CHECKSUM;
    
    // TBD
    return STATUS_SUCCESS;
}