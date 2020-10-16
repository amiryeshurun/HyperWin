#ifndef __COM_VMCALL_HANLDERS_H_
#define __COM_VMCALL_HANLDERS_H_

#include <error_codes.h>
#include <types.h>
#include <vmm/vmm.h>

STATUS HandleVmCallCommunication(IN PCURRENT_GUEST_STATE data);

#endif