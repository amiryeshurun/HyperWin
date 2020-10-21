#ifndef __ERROR_CODES_H_
#define __ERROR_CODES_H_

#include <types.h>

#define STATUS QWORD

#define STATUS_SUCCESS                      0
#define STATUS_FAILURE                      1
#define STATUS_RSDP_NOT_FOUND               2
#define STATUS_RSDP_INVALID_CHECKSUM        3
#define STATUS_NO_MEM_AVAILABLE             4
#define STATUS_RSDT_INVALID_CHECKSUM        5
#define STATUS_APIC_NOT_FOUND               6
#define STATUS_NO_CORES_FOUND               7
#define STATUS_VMLAUNCH_FAILED              8
#define STATUS_MEMORY_NOT_ALIGNED           9
#define STATUS_ACCESS_TO_HIDDEN_BASE        10
#define STATUS_INVALID_ACCESS_RIGHTS        11
#define STATUS_XSETBV_CANCELLED             12
#define STATUS_UNKNOWN_VMCALL               13
#define STATUS_ADDRESS_NOT_VALID            15
#define STATUS_UNKNOWN_OPERAND              16
#define STATUS_UNHANDLED_EPT_VIOLATION      17
#define STATUS_INVALID_GUEST_STATE          18
#define STATUS_INVALID_MSR_LOADING          19
#define STATUS_MACHINE_CHECK_FAILURE        20
#define STATUS_TRIPLE_FAULT                 21
#define STATUS_E820_NOT_HOOKED              22
#define STATUS_CODE_IN_DIFFERENT_REGIONS    23
#define STATUS_CODE_REGION_NOT_FOUND        25
#define STATUS_2XAPIC_NOT_AVAILABLE         26
#define STATUS_APIC_SIPI_FAILED             27
#define STATUS_SIPI_WITHOUT_INIT            28
#define STATUS_COMMUNICATION_PARSING_FAILED 29
#define STATUS_COULD_NOT_ALLOCATE           30
#define STATUS_UNALLOCATED_MEMORY           31
#define STATUS_DEFRAGMENTATION_FAILED       32

#endif