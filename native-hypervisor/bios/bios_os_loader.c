#include <bios/bios_os_loader.h>
#include <vmm/vmm.h>
#include <utils/utils.h>
#include <intrinsics.h>
#include <debug.h>
#include <vmm/vmcs.h>
#include <vmm/vm_operations.h>

BiosFunction functionsBegin[] = { DiskReader, GetMemoryMap, SleepAsm };
BiosFunction functionsEnd[] = { DiskReaderEnd, GetMemoryMapEnd, SleepAsmEnd };

VOID BiosEnterRealModeRunFunction(IN BYTE function, OUT BYTE_PTR* outputBuffer)
{
    BiosFunction functionBegin, functionEnd;
    QWORD enterRealModeLength, functionLength;

    functionBegin = functionsBegin[function];
    functionEnd = functionsEnd[function];
    functionLength = functionEnd - functionBegin;
    enterRealModeLength = EnterRealModeEnd - EnterRealMode;
    HwCopyMemory((BYTE_PTR)REAL_MODE_CODE_START, EnterRealMode, enterRealModeLength);
    HwCopyMemory((BYTE_PTR)REAL_MODE_CODE_START + enterRealModeLength, 
               functionBegin, 
               functionLength);
    AsmEnterRealModeRunFunction();

    if(outputBuffer)
        outputBuffer = (BYTE_PTR)REAL_MODE_OUTPUT_BUFFER_ADDRESS;
}

VOID BiosReadFirstSectorToRam(IN BYTE diskIndex, OUT BYTE_PTR* address)
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
    BiosEnterRealModeRunFunction(DISK_READER, NULL);
    if(!address)
    {
        Print("Output address was not specified as a destenation for int 13h\n");
        ASSERT(FALSE);
    }
    *address = FIRST_SECTOR_DEST;
}

VOID BiosLoadMBRToEntryPoint()
{
    PMBR sectorAddress;

    for(BYTE diskIdx = 0x80; diskIdx < 0xff; diskIdx++)
    {
        BiosReadFirstSectorToRam(diskIdx, &sectorAddress);
        if(*(WORD_PTR)((BYTE_PTR)sectorAddress + MBR_SIZE - sizeof(WORD)) == BOOTABLE_SIGNATURE)
        {
            Print("An MBR disk was found at disk #%d\n", diskIdx);
            HwCopyMemory(MBR_ADDRESS, sectorAddress, MBR_SIZE);
            *(BYTE_PTR)(WINDOWS_DISK_INDEX) = diskIdx;
            break;
        }
    }
    // 0x7c00 now contains the MBR
}

VOID BiosSleep(IN DWORD milliSeconds)
{
    DWORD timeInMs;

    timeInMs = milliSeconds * 1000;
    *(WORD_PTR)SLEEP_TIME_FIRST_2 = timeInMs >> 16;
    *(WORD_PTR)SLEEP_TIME_SECOND_2 = timeInMs & 0xffff;
    BiosEnterRealModeRunFunction(SLEEP, NULL);
}

STATUS BiosFindRSDT(OUT BYTE_PTR* address, OUT QWORD_PTR type)
{
    CHAR pattern[] = "RSD PTR ";
    BYTE_PTR ebdaAddress, rsdpBaseAddress;
    QWORD sum;

    ebdaAddress = (*(WORD_PTR)EBDA_POINTER_ADDRESS) >> 4;
    for(QWORD i = 0; i < 0x1024; i += 16)
    {
        if(!HwCompareMemory(ebdaAddress + i, pattern, 8))
        {
            rsdpBaseAddress = (ebdaAddress + i);
            goto RSDPFound;
        }
    }

    for(QWORD start = 0xE0000; start < 0xFFFFF; start += 16)
    {
        if(!HwCompareMemory(start, pattern, 8))
        {
            rsdpBaseAddress = start;
            goto RSDPFound;
        }
    }
    
    return STATUS_FAILURE;

RSDPFound:

    sum = 0;
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

STATUS BiosLocateSystemDescriptorTable(IN BYTE_PTR rsdt, OUT BYTE_PTR* table, IN QWORD type, IN PCHAR signature)
{
    QWORD sum, entriesCount;
    DWORD_PTR dwSectionsArrayBase;
    QWORD_PTR qwSectionsArrayBase;

    sum = 0;
    for(QWORD i = 0; i < *(DWORD_PTR)(rsdt + RSDT_LENGTH_OFFSET); i++)
        sum += rsdt[i];
    if(sum % 0x100)
        return STATUS_RSDT_INVALID_CHECKSUM;
    if(type == 1) // ACPI Version 1.0
    {
        entriesCount = ((*(DWORD_PTR)(rsdt + RSDT_LENGTH_OFFSET)) - ACPI_SDT_HEADER_SIZE) / 4;
        dwSectionsArrayBase = rsdt + ACPI_SDT_HEADER_SIZE;

        for(QWORD i = 0; i < entriesCount; i++)
        {
            if(!HwCompareMemory(signature, dwSectionsArrayBase[i], 4))
            {
                *table = dwSectionsArrayBase[i];
                return STATUS_SUCCESS;
            }
        }

    }
    else // Version >= 2.0
    {
        entriesCount = ((*(DWORD_PTR)(rsdt + RSDT_LENGTH_OFFSET)) - ACPI_SDT_HEADER_SIZE) / 8;
        qwSectionsArrayBase = rsdt + ACPI_SDT_HEADER_SIZE;
        for(QWORD i = 0; i < entriesCount; i++)
        {
            if(!HwCompareMemory(signature, qwSectionsArrayBase[i], 4))
            {
                *table = qwSectionsArrayBase[i];
                return STATUS_SUCCESS;
            }
        }
    }

    return STATUS_APIC_NOT_FOUND;
}

STATUS BiosHandleE820(IN PCURRENT_GUEST_STATE data, IN PREGISTERS regs)
{
    BOOL carrySet;
    WORD csValue, flags;

    carrySet = FALSE;
    if(regs->rbx >= data->currentCPU->sharedData->memoryRangesCount)
    {
        carrySet = TRUE;
        goto EmulateIRET;
    }
    regs->rax = E820_MAGIC;
    regs->rcx = (regs->rcx & ~(0xffULL)) | 20;
    HwCopyMemory(vmread(GUEST_ES_BASE) + (regs->rdi & 0xffffULL), 
        &(data->currentCPU->sharedData->allRam[regs->rbx++]), sizeof(E820_LIST_ENTRY));
    carrySet = FALSE;
    if(regs->rbx == data->currentCPU->sharedData->memoryRangesCount)
        regs->rbx = 0;
    
EmulateIRET:
    regs->rip = (DWORD)(*(DWORD_PTR)(vmread(GUEST_SS_BASE) + regs->rsp));
    csValue = *(WORD_PTR)(vmread(GUEST_SS_BASE) + regs->rsp + 2);
    flags = *(WORD_PTR)(vmread(GUEST_SS_BASE) + regs->rsp + 4);
    if(carrySet) 
        flags |= RFLAGS_CARRY;
    else
        flags &= ~(RFLAGS_CARRY);
    __vmwrite(GUEST_CS_BASE, csValue << 4);
    __vmwrite(GUEST_CS_SELECTOR, csValue);
    regs->rsp += 6;
    return STATUS_SUCCESS;
}