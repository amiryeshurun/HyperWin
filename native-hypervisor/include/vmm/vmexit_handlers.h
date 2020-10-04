#ifndef __VMEXIT_HANDLERS_H_
#define __VMEXIT_HANDLERS_H_

#include <vmm/vmm.h>

/* VM-Exit handlers */
STATUS EmulateXSETBV(IN PCURRENT_GUEST_STATE data);
STATUS HandleVmCall(IN PCURRENT_GUEST_STATE data);
STATUS HandleMsrRead(IN PCURRENT_GUEST_STATE data);
STATUS HandleMsrWrite(IN PCURRENT_GUEST_STATE data);
STATUS HandleCpuId(IN PCURRENT_GUEST_STATE data);
STATUS HandleEptViolation(IN PCURRENT_GUEST_STATE data);
STATUS HandleInvalidGuestState(IN PCURRENT_GUEST_STATE data);
STATUS HandleInvalidMsrLoading(IN PCURRENT_GUEST_STATE data);
STATUS HandleMachineCheckFailure(IN PCURRENT_GUEST_STATE data);
STATUS HandleCrAccess(IN PCURRENT_GUEST_STATE data);
STATUS HandleTripleFault(IN PCURRENT_GUEST_STATE data);

#endif