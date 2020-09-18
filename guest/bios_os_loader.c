#include <guest/bios_os_loader.h>
#include <host/vmm.h>
#include <utils.h>

BiosFunction functionsBegin[] = { DiskReader, GetMemoryMap };
BiosFunction functionsEnd[] = { DiskReaderEnd, GetMemoryMapEnd };

VOID EnterRealModeRunFunction(IN BYTE function, OUT BYTE_PTR* outputBuffer)
{
    BiosFunction functionBegin = functionsBegin[function];
    BiosFunction functionEnd = functionsEnd[function];
    QWORD enterRealModeLength = EnterRealModeEnd - EnterRealMode;
    QWORD functionLeanth = functionEnd - functionBegin;
    CopyMemory((QWORD_PTR)REAL_MODE_CODE_START, EnterRealMode, enterRealModeLength);
    CopyMemory((QWORD_PTR)REAL_MODE_CODE_START + enterRealModeLength, 
               functionBegin, 
               functionLeanth);
    
    AsmEnterRealModeRunFunction();

    if(outputBuffer != NULL)
        outputBuffer = (BYTE_PTR)REAL_MODE_OUTPUT_BUFFER_ADDRESS;
}

VOID ReadFirstSectorToRam(IN BYTE diskIndex, OUT BYTE_PTR* address)
{
    PDISK_ADDRESS_PACKET packet = DAP_ADDRESS;
    packet->size = 0x10;
    packet->reserved = 0;
    packet->count = 1;
    packet->offset = 0;
    packet->dest = FIRST_SECTOR_DEST;
    packet->sectorNumberLowPart = 0;
    packet->sectorNumberHighPart = 0;
    CopyMemory(DAP_ADDRESS + sizeof(DISK_ADDRESS_PACKET), &diskIndex, sizeof(BYTE));

    EnterRealModeRunFunction(DISK_READER, NULL);
    *address = FIRST_SECTOR_DEST;
}

VOID LoadMBRToEntryPoint()
{
    PMBR sectorAddress;

    for(BYTE diskIdx = 0x80; diskIdx < 0xff; diskIdx++)
    {
        ReadFirstSectorToRam(diskIdx, &sectorAddress);
        if(sectorAddress->magic == BOOTABLE_SIGNATURE)
        {
            CopyMemory(MBR_ADDRESS, sectorAddress, sizeof(MBR));
            *(BYTE_PTR)(WINDOWS_DISK_INDEX) = diskIdx;
            break;
        }
    }
    // 0x7c00 now contains the MBR
}
