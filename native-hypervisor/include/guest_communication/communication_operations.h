#ifndef __COMMUNICATION_OPERATIONS_H_
#define __COMMUNICATION_OPERATIONS_H_

#include <types.h>

/* Communication block different operations */
typedef QWORD OPERATION, *POPERATION;

#define OPERATION_INIT              0x4857494e4954    // HWINIT ASCII
#define OPERATION_PROTECTED_PROCESS 0x70726f74656374  // PROTECT ASCII
#define OPERATION_COMPLETED         0x444f4e45        // DONE ASCII

#endif