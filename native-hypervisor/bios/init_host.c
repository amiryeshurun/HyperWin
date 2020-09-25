#include <utils.h>
#include <bios/bios_os_loader.h>
#include <vmm/vmm.h>
#include <intrinsics.h>
#include <debug.h>

extern VOID UpdateInstructionPointer(QWORD offset);

VOID Initialize()
{
    SetupVirtualAddress(__readcr3());
    InitializeHypervisorsSharedData(CODE_BEGIN_ADDRESS, 0x000fffffULL);
    LoadMBRToEntryPoint();
    CopyMemory((QWORD_PTR)REAL_MODE_CODE_START, (QWORD_PTR)SetupSystemAndHandleControlToBios,
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
    allocationSize += ALIGN_UP(codeBase, PAGE_SIZE);
    allocationSize += ALIGN_UP(sizeof(SHARED_CPU_DATA), PAGE_SIZE);
    allocationSize += (ALIGN_UP(sizeof(SINGLE_CPU_DATA), PAGE_SIZE) * numberOfCores);
    allocationSize += (ALIGN_UP(sizeof(CURRENT_GUEST_STATE) * numberOfCores, PAGE_SIZE));
    BYTE_PTR physicalHypervisorBase;
    if(AllocateMemoryUsingMemoryMap(memoryMap, memoryRegionsCount, allocationSize, &physicalHypervisorBase))
    {
        Print("Allocation of %8 bytes failed.\n", allocationSize);
        ASSERT(FALSE);
    }
    QWORD hypervisorBase = PhysicalToVirtual(physicalHypervisorBase);
    SetMemory(hypervisorBase, 0, allocationSize);
    CopyMemory(hypervisorBase, codeBase, codeBase);
    UpdateInstructionPointer(hypervisorBase - codeBase);
    PSHARED_CPU_DATA sharedData = hypervisorBase + ALIGN_UP(codeBase, PAGE_SIZE);
    sharedData->numberOfCores = numberOfCores;
    for(BYTE i = 0; i < numberOfCores; i++)
    {
        sharedData->cpuData[i] = hypervisorBase + ALIGN_UP(codeBase, PAGE_SIZE) 
            + ALIGN_UP(sizeof(SHARED_CPU_DATA), PAGE_SIZE) + i * ALIGN_UP(sizeof(SINGLE_CPU_DATA), PAGE_SIZE);
        sharedData->currentState[i] = hypervisorBase + ALIGN_UP(codeBase, PAGE_SIZE) 
            + ALIGN_UP(sizeof(SHARED_CPU_DATA), PAGE_SIZE) + numberOfCores * ALIGN_UP(sizeof(SINGLE_CPU_DATA), PAGE_SIZE)
            + i * sizeof(CURRENT_GUEST_STATE);
        sharedData->currentState[i] = sharedData->cpuData[i];
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
    InitializeSingleHypervisor(sharedData->cpuData[0]);
}

STATUS AllocateMemoryUsingMemoryMap
    (IN PE820_LIST_ENTRY memoryMap, IN DWORD memoryRegionsCount, IN QWORD allocationSize, OUT BYTE_PTR* address)
{
    QWORD alignedAllocationSize = ALIGN_UP(allocationSize, LARGE_PAGE_SIZE);
    INT upperIdx = NEG_INF; // high addresses are more rarely to be in use on the computer startup, use them
    for(DWORD i = 0; i < memoryRegionsCount; i++)
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
    QWORD unalignedCount = memoryMap[upperIdx].baseAddress % PAGE_SIZE;
    memoryMap[upperIdx].length -= (alignedAllocationSize + unalignedCount);
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
        if(!CompareMemory(ebdaAddress + i, pattern, 8))
        {
            rsdpBaseAddress = (ebdaAddress + i);
            goto RSDPFound;
        }
    }

    for(QWORD start = 0xE0000; start < 0xFFFFF; start += 16)
    {
        if(!CompareMemory(start, pattern, 8))
        {
            rsdpBaseAddress = start;
            goto RSDPFound;
        }
    }
    
    return STATUS_FAILURE;

RSDPFound:
    NOP
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
    *address = *(QWORD_PTR)(rsdpBaseAddress + RSDP_STRUCTURE_SIZE + 4); // Xsdt
    return STATUS_SUCCESS;
}

STATUS LocateSystemDescriptorTable(IN BYTE_PTR rsdt, OUT BYTE_PTR* table, IN QWORD type, IN PCHAR signature)
{
    QWORD sum = 0;
    for(QWORD i = 0; i < *(DWORD_PTR)(rsdt + RSDT_LENGTH_OFFSET); i++)
        sum += rsdt[i];
    if(sum % 0x100)
        return STATUS_RSDT_INVALID_CHECKSUM;
    QWORD entriesCount;
    if(type == 1) // ACPI Version 1.0
    {
        entriesCount = ((*(DWORD_PTR)(rsdt + RSDT_LENGTH_OFFSET)) - ACPI_SDT_HEADER_SIZE) / 4;
        DWORD_PTR sectionsArrayBase = rsdt + ACPI_SDT_HEADER_SIZE;

        for(QWORD i = 0; i < entriesCount; i++)
        {
            if(!CompareMemory(signature, sectionsArrayBase[i], 4))
            {
                *table = sectionsArrayBase[i];
                return STATUS_SUCCESS;
            }
        }

    }
    else // Version >= 2.0
    {
        entriesCount = ((*(DWORD_PTR)(rsdt + RSDT_LENGTH_OFFSET)) - ACPI_SDT_HEADER_SIZE) / 8;
        QWORD_PTR sectionsArrayBase = rsdt + ACPI_SDT_HEADER_SIZE;
        for(QWORD i = 0; i < entriesCount; i++)
        {
            if(!CompareMemory(signature, sectionsArrayBase[i], 4))
            {
                *table = sectionsArrayBase[i];
                return STATUS_SUCCESS;
            }
        }
    }

    return STATUS_APIC_NOT_FOUND;
}

STATUS GetCoresData(IN BYTE_PTR apicTable, OUT BYTE_PTR processorsCount, OUT BYTE_PTR processorsIdentifiers)
{
    QWORD tableLength = *(DWORD_PTR)(apicTable + RSDT_LENGTH_OFFSET);
    *processorsCount = 0;
    for(QWORD offset = 0x2C; offset < tableLength;)
    {
        switch(*(BYTE_PTR)(apicTable + offset))
        {
            case PROCESSOR_LOCAL_APIC:
                processorsIdentifiers[(*processorsCount)++] = *((BYTE_PTR)apicTable + offset + 2);
                break;
            /* reserved for future usage */
        }
        offset += *((BYTE_PTR)apicTable + offset + 1);
    }

    if(!(*processorsCount))
        return STATUS_NO_CORES_FOUND;
    
    return STATUS_SUCCESS;
}

VOID PrintMemoryRanges(IN PE820_LIST_ENTRY start, IN QWORD count)
{
    for(QWORD i = 0; i < count; i++)
        Print("######### %d\n"
               "Base address: %8\n"
               "Length: %d\n"
               "Type: %d\n", i, start[i].baseAddress, start[i].length, start[i].type);
}