#ifndef __DEBUG_H_
#define __DEBUG_H_

#include <types.h>

#define COM1 0x3F8
#define COM2 0x2F8
#define COM3 0x3E8
#define COM4 0x2E8
#define DEBUG_PORT COM3

#define BUFF_MAX_SIZE 0x1000

VOID PrintBuffer(IN PCHAR buffer, IN QWORD length);
VOID PrintNullTerminatedBuffer(IN PCHAR buffer);
VOID Print(IN PCHAR fmt, ...);


#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define ASSERT(condition) \
        do \
        { \
            if(!(condition)) \
            { \
                Print("\n\n\nASSERTION FAILED AT: " STR(__FILE__) ", line: " STR(__LINE__) ", condition: [[[ " STR(condition) " ]]]" "\n\n\n"); \
                for(;;); \
            } \
        } while(FALSE)


#endif