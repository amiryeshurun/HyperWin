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

extern VOID ApicStart();
extern VOID ApicEnd();

STATUS DetectX2APICAvailability();
VOID EnableX2APIC();
STATUS GetCoresData(IN BYTE_PTR apicTable, OUT BYTE_PTR processorsCount, OUT BYTE_PTR processorsIdentifiers);
VOID IssueIPI(IN QWORD destenation, IN QWORD vector, IN QWORD deliveryMode, IN QWORD lvl);
STATUS ActivateHypervisorOnProcessor(IN QWORD processorId, IN PSINGLE_CPU_DATA cpuData);

#endif