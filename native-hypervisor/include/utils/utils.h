#ifndef __UTILS_H_
#define __UTILS_H_

#include <types.h>
#include <debug.h>
#include <utils/string.h>
#include <error_codes.h>

#define ALIGN_UP(x, y) (((x) + ((y) - 1)) & ~((y) - 1))
#define ALIGN_DOWN(x, y) ((x) & (~((y) - 1)))

#define FIELD_OFFSET(type, field) (&((type)(0)->field))

#define INF 0xffffffffffffffffULL
// Should be enough
#define NEG_INF (-0xffff)

#define BASIC_HASH_LEN 11
#define IDX_NOT_FOUND (-1ULL)
#define VALUE_NOT_FOUND (-1ULL)

// Check for success or return
#define SUCCESS_OR_RETURN(expression) \
                    if((status = expression) != STATUS_SUCCESS) \
                        return status

#define SUCCESS_OR_CLEANUP(expression) \
                    do \
                    { \
                        if((status = expression) != STATUS_SUCCESS) \
                        { \
                            Print("%d: Non-successful status returned at: " STR(__FILE__) ", line: " STR(__LINE__) ", condition: [[[ " STR(expression) " ]]]" "\n", status); \
                            goto cleanup; \
                        } \
                    } while(FALSE)

// Spinlock (for multiprocessing)
typedef struct _SPIN_LOCK
{
	volatile QWORD lockValue;
} SPIN_LOCK, *PSPIN_LOCK;

#define SPIN_LOCK_INIT { 0 }
#define SPIN_LOCK(slock) while(!__sync_bool_compare_and_swap(&((slock)->lockValue), 0, 1))
#define SPIN_UNLOCK(slock) (slock)->lockValue = 0
#define IS_LOCKED(slock) (((slock)->lockValue == 1) ? TRUE : FALSE)

VOID HwCopyMemory(OUT BYTE_PTR dest, IN BYTE_PTR src, IN QWORD count);
INT HwCompareMemory(IN BYTE_PTR buff1, IN BYTE_PTR buff2, IN QWORD length);
CHAR ConvertHalfByteToHexChar(IN BYTE halfByte);
QWORD pow(IN QWORD base, IN QWORD power);
QWORD NumberOfDigits(IN QWORD number);
QWORD StringLength(IN PCHAR str);
VOID HwSetMemory(IN BYTE_PTR base, IN BYTE value, IN QWORD length);
VOID DumpHostStack(IN QWORD_PTR stackAddress);
BOOL IsMsrValid(IN QWORD msrNumber, IN BYTE_PTR msrRange);
QWORD SumDigits(IN QWORD num);
QWORD MemoryContains(IN BYTE_PTR buff1, IN QWORD size1, IN BYTE_PTR buff2, IN QWORD size2,
    OUT QWORD_PTR indecies);
QWORD GetTokenLength(IN BYTE_PTR begin, IN CHAR separator);
QWORD StringToInt(IN PCHAR str, IN QWORD strlen);

#endif
