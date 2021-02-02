#ifndef __ERROR_CODES_H_
#define __ERROR_CODES_H_

#include <types.h>

#define STATUS QWORD

#define STATUS_SUCCESS                         0
#define STATUS_FAILURE                         1
#define STATUS_RSDP_NOT_FOUND                  2
#define STATUS_RSDP_INVALID_CHECKSUM           3
#define STATUS_NO_MEM_AVAILABLE                4
#define STATUS_RSDT_INVALID_CHECKSUM           5
#define STATUS_APIC_NOT_FOUND                  6
#define STATUS_NO_CORES_FOUND                  7
#define STATUS_VMLAUNCH_FAILED                 8
#define STATUS_MEMORY_NOT_ALIGNED              9
#define STATUS_ACCESS_TO_HIDDEN_BASE           10
#define STATUS_INVALID_ACCESS_RIGHTS           11
#define STATUS_XSETBV_CANCELLED                12
#define STATUS_UNKNOWN_VMCALL                  13
#define STATUS_ADDRESS_NOT_VALID               15
#define STATUS_UNKNOWN_OPERAND                 16
#define STATUS_UNHANDLED_EPT_VIOLATION         17
#define STATUS_INVALID_GUEST_STATE             18
#define STATUS_INVALID_MSR_LOADING             19
#define STATUS_MACHINE_CHECK_FAILURE           20
#define STATUS_TRIPLE_FAULT                    21
#define STATUS_E820_NOT_HOOKED                 22
#define STATUS_CODE_IN_DIFFERENT_REGIONS       23
#define STATUS_CODE_REGION_NOT_FOUND           25
#define STATUS_2XAPIC_NOT_AVAILABLE            26
#define STATUS_APIC_SIPI_FAILED                27
#define STATUS_SIPI_WITHOUT_INIT               28
#define STATUS_COMMUNICATION_PARSING_FAILED    29
#define STATUS_HEAP_FULL                       30
#define STATUS_UNALLOCATED_MEMORY              31
#define STATUS_DEFRAGMENTATION_FAILED          32
#define STATUS_INVALID_MSR                     33
#define STATUS_NO_SPACE_AVAILABLE              34
#define STATUS_UNKNOWN_KPP_INSTRUCTION         35
#define STATUS_SSDT_NOT_FOUND                  36
#define STATUS_ERROR_CODE_MUST_BE_SPECIFIED    37
#define STATUS_OBJECT_FIELD_NOT_FOUND          38
#define STATUS_ETHREAD_NOT_AVAILABLE           39
#define STATUS_COULD_NOT_TRANSLATE_HANDLE      40
#define STATUS_PROTECTED_PROCESS_FAILED        41
#define STATUS_UNKNOWN_COMMUNICATION_OPERATION 43
#define STATUS_KEY_ALREADY_EXISTS              44
#define STATUS_UNKNOWN_VMCALL_CALLER           45
#define STATUS_MODULE_NOT_FOUND                46
#define STATUS_SYSCALL_NOT_HANDLED             47
#define STATUS_GROUP_ALREADY_EXISTS            48
#define STATUS_FILE_NOT_FOUND                  49
#define STATUS_INSTRUCTION_TOO_SHORT           50
#define STATUS_UNKNOWN_HOOK_TYPE               51
#define STATUS_UNKNOWN_HOOK_NAME               52
#define STATUS_SYSCALL_NOT_FOUND               53
#define STATUS_UNKNOWN_HOOK_ADDRESS            54
#define STATUS_MDL_ADDRESS_INVALID             55

#define STATUS_VM_EXIT_NOT_HANDLED             (1 << 16)

#endif