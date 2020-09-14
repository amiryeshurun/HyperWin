#ifndef __HOST_MEMORY_MANAGER_H_
#define __HOST_MEMORY_MANAGER_H_

#include <types.h>

QWORD VirtualToPhysical(IN QWORD address);
QWORD PhysicalToVirtual(IN QWORD address);

#endif