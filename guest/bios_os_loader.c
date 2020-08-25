#include <guest/bios_os_loader.h>
#include <util.h>

VOID ReadFirstSectorToRam(IN BYTE diskIndex, OUT BYTE_PTR address)
{
    PDISK_ADDRESS_PACKET packet = LBA_ADDRESS;
    packet->size = 0x10;
    packet->reserved = 0;
    packet->count = 1;
    packet->offset = 0;
    packet->dest = FIRST_SECTOR_DEST;
    packet->sectorNumberLowPart = 0;
    packet->sectorNumberHighPart = 0;
    CopyMemory(LBA_ADDRESS + sizeof(DISK_ADDRESS_PACKET), &diskIndex, sizeof(BYTE));

    DiskReader();
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
            break;
        }
    }
    // 0x7c00 now contains the MBR
}
