#include <types.h>

typedef VOID (*BiosFunction)();

extern VOID ExitLongMode();
extern VOID ExitLongModeEnd();
extern VOID BootloaderDiskReader();
extern VOID BootloaderDiskReaderEnd();
extern VOID BootloaderAsmEnterRealModeRunFunction();
extern VOID BootloaderAsmEnterRealModeRunFunctionEnd();
extern VOID BootloaderEnterRealMode();
extern VOID BootloaderEnterRealModeEnd();

#define REAL_MODE_CODE_START 0x4200
#define REAL_MODE_OUTPUT_BUFFER_ADDRESS 0x2200
#define HYPERVISOR_CODE_BASE 0x3300000
#define HYPERVISOR_CODE_LENGTH 0x000fffffULL
#define DAP_ADDRESS 0x1200
#define DISK_INDEX_ADDRESS 0x1550
#define SECTOR_SIZE 512

typedef struct _DISK_ADDRESS_PACKET 
{
    BYTE size;
    BYTE reserved;
    WORD count;
    WORD offset;
    WORD segment;
    DWORD sectorNumberLowPart;
    DWORD sectorNumberHighPart;
} __attribute__((__packed__)) DISK_ADDRESS_PACKET, *PDISK_ADDRESS_PACKET;

VOID BootloaderCopyMemory(BYTE_PTR dest, BYTE_PTR src, QWORD length)
{
    for(QWORD i = 0; i < length; i++)
        dest[i] = src[i];
}

VOID BootloaderEnterRealModeRunFunction(IN BiosFunction functionBegin, IN BiosFunction functionEnd,
    OUT BYTE_PTR* outputBuffer)
{
    QWORD enterRealModeLength = BootloaderEnterRealModeEnd - BootloaderEnterRealMode;
    QWORD functionLength = functionEnd - functionBegin;
    BootloaderCopyMemory((BYTE_PTR)REAL_MODE_CODE_START, BootloaderEnterRealMode, enterRealModeLength);
    BootloaderCopyMemory((BYTE_PTR)REAL_MODE_CODE_START + enterRealModeLength, 
               functionBegin, 
               functionLength);
    BootloaderAsmEnterRealModeRunFunction();

    if(outputBuffer)
        outputBuffer = (BYTE_PTR)REAL_MODE_OUTPUT_BUFFER_ADDRESS;
}

VOID BootloaderReadSector(IN BYTE diskIndex, IN QWORD sectorNumber, OUT BYTE_PTR* address)
{
    PDISK_ADDRESS_PACKET packet = DAP_ADDRESS;
    packet->size = 0x10;
    packet->reserved = 0;
    packet->count = 1;
    packet->offset = REAL_MODE_OUTPUT_BUFFER_ADDRESS;
    packet->segment = 0;
    packet->sectorNumberLowPart = sectorNumber & 0xffffffffU;
    packet->sectorNumberHighPart = sectorNumber >> 32;
    *(BYTE_PTR)(DAP_ADDRESS + 0x10) = diskIndex;
    BootloaderEnterRealModeRunFunction(BootloaderDiskReader, BootloaderDiskReaderEnd, NULL);
    *address = REAL_MODE_OUTPUT_BUFFER_ADDRESS;
}

VOID BootloaderSectorsLoader()
{
    BYTE diskIndex = *(BYTE_PTR)DISK_INDEX_ADDRESS;
    QWORD lengthInSectors = HYPERVISOR_CODE_LENGTH / SECTOR_SIZE + 1,
        firstSector = HYPERVISOR_CODE_BASE / SECTOR_SIZE;
    BYTE_PTR dest = HYPERVISOR_CODE_BASE, realModeDest;

    for(QWORD sector = firstSector; sector <= firstSector + lengthInSectors; sector++)
    {
        BootloaderReadSector(diskIndex, sector, &dest);
        BootloaderCopyMemory(dest, realModeDest, SECTOR_SIZE);
        dest += SECTOR_SIZE;
    }
}