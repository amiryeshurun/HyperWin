#ifndef __BIOS_OS_LOADER_H_
#define __BIOS_OS_LOADER_H_

#include <types.h>
#include <utils/utils.h>
#include <error_codes.h>

#define MBR_ADDRESS 0x7C00
#define DAP_ADDRESS 0x4000
#define FIRST_SECTOR_DEST 0x3000
#define BOOTABLE_SIGNATURE 0xAA55
#define REAL_MODE_OUTPUT_BUFFER_ADDRESS 0x2200
#define REAL_MODE_CODE_START 0x4200
#define WINDOWS_DISK_INDEX 0x6010
#define CODE_BEGIN_ADDRESS 0x3300000
#define SLEEP_TIME_FIRST_2 0x4000
#define SLEEP_TIME_SECOND_2 0x4002
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

/* E820 related dat */
#define E820_USABLE_REGION 1
#define E820_RESERVED_REGION 2
#define E820_RECLAIMABLE_REGION 3
#define E820_NVS_REGION 4
#define E820_BAD_MEMORY_REGION 5
#define E820_OUTPUT_MAX_ENTRIES 30
#define E820_OUTPUT_ADDRESS 0x8600
#define E820_VMCALL_GATE 0xfffc
#define E820_MAGIC 0x534D4150

enum{
    DISK_READER = 0,
    GET_MEMORY_MAP = 1,
    SLEEP = 2
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

typedef struct _E820_LIST_ENTRY
{
    QWORD baseAddress;
    QWORD length;
    DWORD type;
    DWORD extendedAttribute;
} __attribute__((__packed__)) E820_LIST_ENTRY, *PE820_LIST_ENTRY;

typedef union _INTERRUPT_COMMAND_REGISTER
{
    QWORD full;
    struct
    {
        QWORD vector : 8;
        QWORD deliveryMode : 3;
        QWORD destinationMode : 1;
        QWORD deliveryStatus : 1;
        QWORD reserved0 : 1;
        QWORD level : 1;
        QWORD triggerMode : 1;
        QWORD reserved1 : 2;
        QWORD destinationShort : 2;
        QWORD reserved2 : 35;
        QWORD destination : 8;
    } bitFields;
}INTERRUPT_COMMAND_REGISTER, *PINTERRUPT_COMMAND_REGISTER;;

typedef VOID (*BiosFunction)();

extern VOID DiskReader();
extern VOID DiskReaderEnd();
extern VOID EnterRealMode();
extern VOID EnterRealModeEnd();
extern VOID AsmEnterRealModeRunFunction();
extern VOID GetMemoryMap();
extern VOID GetMemoryMapEnd();
extern VOID SetupSystemAndHandleControlToBios();
extern VOID SetupSystemAndHandleControlToBiosEnd();
extern VOID SleepAsm();
extern VOID SleepAsmEnd();

VOID EnterRealModeRunFunction(IN BYTE function, OUT BYTE_PTR* outputBuffer);
VOID ReadFirstSectorToRam(IN BYTE diskIndex, OUT BYTE_PTR* address);
VOID LoadMBRToEntryPoint();
STATUS FindRSDT(OUT BYTE_PTR* address, OUT QWORD_PTR type);
STATUS LocateSystemDescriptorTable(IN BYTE_PTR rsdt, OUT BYTE_PTR* table, IN QWORD type, IN PCHAR signature);
STATUS AllocateMemoryUsingMemoryMap
    (IN PE820_LIST_ENTRY memoryMap, IN DWORD memoryRegionsCount, IN QWORD allocationSize, OUT BYTE_PTR* address);
VOID PrintMemoryRanges(IN PE820_LIST_ENTRY start, IN QWORD count);
STATUS HideCodeBase(IN PE820_LIST_ENTRY memoryMap, OUT WORD_PTR updatedCount, IN QWORD codeBegin, IN QWORD codeLength);
VOID Sleep(IN DWORD milliSeconds);

struct _CURRENT_GUEST_STATE;
STATUS HandleE820(IN struct _CURRENT_GUEST_STATE* data, IN PREGISTERS regs);

#endif