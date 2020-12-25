#ifndef __APIC_H_
#define __APIC_H_

#include <types.h>
#include <error_codes.h>
#include <vmm/vmm.h>

#define APIC_FUNC_ADDRESS 0x8000
#define CPU_DATA_ADDRESS 0x7c00
#define SEMAPHORE_LOCATION 0x4020

/* APIC related data */
#define LOCAL_APIC_VERSION_REGISTER_ADDRESS 0xfee00030
#define APIC_SIPI_INTERRUPT (6 << 8)
#define APIC_INIT_INTERRUPT (5 << 8)
#define APIC_DEST_MODE_PHYSICAL (0 << 10)
#define APIC_DESTINAION_BIT_OFFSET 32
#define APIC_LEVEL_DEASSERT (0 << 14)
#define APIC_LEVEL_ASSERT (1 << 14)

/* State values */
#define CPU_STATE_ACTIVE 0
#define CPU_STATE_HLT 1
#define CPU_STATE_SHUTDOWN 2
#define CPU_STATE_WAIT_FOR_SIPI 3

extern VOID ApicStart();
extern VOID ApicEnd();

STATUS ApicDetectX2APICAvailability();
VOID ApicEnableX2APIC();
VOID ApicEnableAPIC();
STATUS ApicGetCoresData(IN BYTE_PTR apicTable, OUT BYTE_PTR processorsCount, OUT BYTE_PTR processorsIdentifiers);
VOID ApicX2APICIssueIPI(IN QWORD destenation, IN QWORD vector, IN QWORD deliveryMode, IN QWORD lvl);
STATUS ApicActivateHypervisorOnProcessor(IN QWORD processorId, IN PSINGLE_CPU_DATA cpuData);
VOID ApicGetBaseAddress();

#endif