#ifndef __APIC_H_
#define __APIC_H_

#include <types.h>
#include <error_codes.h>
#include <vmm/vmm.h>

#define APIC_FUNC_ADDRESS 0x7c00

/* APIC related data */
#define LOCAL_APIC_VERSION_REGISTER_ADDRESS 0xfee00030
#define APIC_SIPI_INTERRUPT (6 << 7)
#define APIC_INIT_INTERRUPT (5 << 7)
#define APIC_DEST_MODE_PHYSICAL (0 << 10)
#define APIC_DESTINAION_BIT_OFFSET 32

extern VOID ApicStart();
extern VOID ApicEnd();

STATUS DetectX2APICAvailability();
VOID EnableX2APIC();
STATUS GetCoresData(IN BYTE_PTR apicTable, OUT BYTE_PTR processorsCount, OUT BYTE_PTR processorsIdentifiers);
VOID IssueIPI(IN QWORD destenation, IN QWORD vector, IN QWORD deliveryMode);
STATUS ActivateHypervisorOnProcessor(IN QWORD processorId, IN PSINGLE_CPU_DATA cpuData);

#endif