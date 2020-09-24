#ifndef __ERROR_CODES_H_
#define __ERROR_CODES_H_

#include <types.h>


#define STATUS QWORD
#define STATUS_SUCCESS               0
#define STATUS_FAILURE               1
#define STATUS_RSDP_NOT_FOUND        2
#define STATUS_RSDP_INVALID_CHECKSUM 3
#define STATUS_NO_MEM_AVAILABLE      4
#define STATUS_RSDT_INVALID_CHECKSUM 5
#define STATUS_APIC_NOT_FOUND        6
#define STATUS_NO_CORES_FOUND        7

#endif