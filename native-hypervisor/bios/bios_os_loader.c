#include <bios/bios_os_loader.h>
#include <vmm/vmm.h>
#include <utils.h>
#include <intrinsics.h>
#include <debug.h>

BiosFunction functionsBegin[] = { DiskReader, GetMemoryMap };
BiosFunction functionsEnd[] = { DiskReaderEnd, GetMemoryMapEnd };

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
