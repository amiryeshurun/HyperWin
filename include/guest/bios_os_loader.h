#ifndef __BIOS_OS_LOADER_H_
#define __BIOS_OS_LOADER_H_

#include <types.h>

#define MBR_ADDRESS 0x7c00
#define LBA_ADDRESS 0x4000
#define FIRST_SECTOR_DEST 0x3000
#define BOOTABLE_SIGNATURE 0x55aa

extern VOID DiskReader();

typedef struct _DISK_ADDRESS_PACKET
{
    BYTE size;
    BYTE reserved;
    WORD count;
    WORD offset;
    WORD dest;
    DWORD sectorNumberLowPart;
    DWORD sectorNumberHighPart;
}DISK_ADDRESS_PACKET, *PDISK_ADDRESS_PACKET;

typedef struct _PARTITION_TABLE_ENTRY
{
    BYTE status;
    BYTE CHSFirstAddress[3];
    BYTE type;
    BYTE CHSLastAddress[3];
    DWORD firstSectorLBA;
    DWORD sectorsCount;
}PARTITION_TABLE_ENTRY, *PPARTITION_TABLE_ENTRY;

typedef struct _MBR
{
    BYTE bootCode[440];
    DWORD diskIndex;
    WORD reserved;
    PARTITION_TABLE_ENTRY partitionTable[4];
    WORD magic;
}MBR, *PMBR;

VOID ReadFirstSectorToRam(IN BYTE diskIndex, OUT BYTE_PTR address);
VOID LoadMBRToEntryPoint();

#endif