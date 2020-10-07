#include <bios/bios_os_loader.h>
#include <vmm/vmm.h>
#include <utils.h>
#include <intrinsics.h>
#include <debug.h>

BiosFunction functionsBegin[] = { DiskReader, GetMemoryMap, SleepAsm };
BiosFunction functionsEnd[] = { DiskReaderEnd, GetMemoryMapEnd, SleepAsmEnd };

VOID EnterRealModeRunFunction(IN BYTE function, OUT BYTE_PTR* outputBuffer)
{
    BiosFunction functionBegin = functionsBegin[function];
    BiosFunction functionEnd = functionsEnd[function];
    QWORD enterRealModeLength = EnterRealModeEnd - EnterRealMode;
    QWORD functionLength = functionEnd - functionBegin;
    CopyMemory((BYTE_PTR)REAL_MODE_CODE_START, EnterRealMode, enterRealModeLength);
    CopyMemory((BYTE_PTR)REAL_MODE_CODE_START + enterRealModeLength, 
               functionBegin, 
               functionLength);
    AsmEnterRealModeRunFunction();

    if(outputBuffer)
        outputBuffer = (BYTE_PTR)REAL_MODE_OUTPUT_BUFFER_ADDRESS;
}

VOID ReadFirstSectorToRam(IN BYTE diskIndex, OUT BYTE_PTR* address)
{
    PDISK_ADDRESS_PACKET packet = DAP_ADDRESS;
    packet->size = 0x10;
    packet->reserved = 0;
    packet->count = 1;
    packet->offset = FIRST_SECTOR_DEST;
    packet->segment = 0;
    packet->sectorNumberLowPart = 0;
    packet->sectorNumberHighPart = 0;
    *(BYTE_PTR)(DAP_ADDRESS + 0x10) = diskIndex;
    EnterRealModeRunFunction(DISK_READER, NULL);
    if(!address)
    {
        Print("Output address was not specified as a destenation for int 13h\n");
        ASSERT(FALSE);
    }
    *address = FIRST_SECTOR_DEST;
}

VOID LoadMBRToEntryPoint()
{
    PMBR sectorAddress;

    for(BYTE diskIdx = 0x80; diskIdx < 0xff; diskIdx++)
    {
        ReadFirstSectorToRam(diskIdx, &sectorAddress);
        if(*(WORD_PTR)((BYTE_PTR)sectorAddress + MBR_SIZE - sizeof(WORD)) == BOOTABLE_SIGNATURE)
        {
            Print("An MBR disk was found at disk #%d\n", diskIdx);
            CopyMemory(MBR_ADDRESS, sectorAddress, MBR_SIZE);
            *(BYTE_PTR)(WINDOWS_DISK_INDEX) = diskIdx;
            break;
        }
    }
    // 0x7c00 now contains the MBR
}

VOID Sleep(IN DWORD milliSeconds)
{
    DWORD timeInMs = milliSeconds * 1000;
    *(WORD_PTR)SLEEP_TIME_FIRST_2 = timeInMs >> 16;
    *(WORD_PTR)SLEEP_TIME_SECOND_2 = timeInMs & 0xffff;
    EnterRealModeRunFunction(SLEEP, NULL);
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