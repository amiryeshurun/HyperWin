#ifndef __BIOS_OS_LOADER_H_
#define __BIOS_OS_LOADER_H_

#include <types.h>
#include <utils.h>
#include <error_codes.h>

#define MBR_ADDRESS 0x7C00
#define DAP_ADDRESS 0x4000
#define FIRST_SECTOR_DEST 0x3000
#define BOOTABLE_SIGNATURE 0xAA55
#define REAL_MODE_OUTPUT_BUFFER_ADDRESS 0x2200
#define REAL_MODE_CODE_START 0x4200
#define WINDOWS_DISK_INDEX 0x6010

#define MBR_SIZE 512 


/* RSDP related data */
#define EBDA_POINTER_ADDRESS 0x040E
#define RSDP_CHECKSUM_OFFSET 0x8
#define RSDP_REVISION_OFFSET 0xF
#define RSDP_ADDRESS_OFFSET 0x10
#define RSDP_STRUCTURE_SIZE 20
#define RSDP_EXTENSION_SIZE 16

/* RSDT related data */
#define RSDT_LENGTH_OFFSET 4
#define ACPI_SDT_HEADER_SIZE 36

/* MADT related data */
#define PROCESSOR_LOCAL_APIC 0
#define IO_APIC 1
#define INTERRUPT_SOURCE_OVERRIDE 2
#define NON_MASKABLE_INTERRUPTS 4
#define LOCAL_APIC_ADDRESS_OVERRIDE 5

enum{
    DISK_READER = 0,
    GET_MEMORY_MAP = 1
};

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

typedef struct _PARTITION_TABLE_ENTRY
{
    BYTE status;
    BYTE CHSFirstAddress[3];
    BYTE type;
    BYTE CHSLastAddress[3];
    DWORD firstSectorLBA;
    DWORD sectorsCount;
} __attribute__((__packed__)) PARTITION_TABLE_ENTRY, *PPARTITION_TABLE_ENTRY;

typedef struct _MBR
{
    BYTE bootCode[440];
    DWORD diskIndex;
    WORD reserved;
    PARTITION_TABLE_ENTRY partitionTable[4];
    WORD magic;
}__attribute__((__packed__))  MBR, *PMBR;

typedef VOID (*BiosFunction)();

extern VOID DiskReader();
extern VOID DiskReaderEnd();
extern VOID EnterRealMode();
extern VOID EnterRealModeEnd();
extern VOID AsmEnterRealModeRunFunction();
extern VOID GetMemoryMap();
extern VOID GetMemoryMapEnd();

VOID EnterRealModeRunFunction(IN BYTE function, OUT BYTE_PTR* outputBuffer);
VOID ReadFirstSectorToRam(IN BYTE diskIndex, OUT BYTE_PTR* address);
VOID LoadMBRToEntryPoint();
STATUS FindRSDT(OUT BYTE_PTR* address, OUT QWORD_PTR type);
STATUS LocateSystemDescriptorTable(IN BYTE_PTR rsdt, OUT BYTE_PTR* table, IN QWORD type, IN PCHAR signature);
STATUS GetCoresData(IN BYTE_PTR apicTable, OUT BYTE_PTR processorsCount, OUT BYTE_PTR processorsIdentifiers);

#endif