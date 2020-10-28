#ifndef __VMEXIT_HANDLERS_H_
#define __VMEXIT_HANDLERS_H_

#include <vmm/vmm.h>
#include <vmx_modules/module.h>

/* VM-Exit handlers */
STATUS EmulateXSETBV(IN PCURRENT_GUEST_STATE data, IN PMODULE module);
STATUS HandleVmCall(IN PCURRENT_GUEST_STATE data, IN PMODULE module);
STATUS HandleMsrRead(IN PCURRENT_GUEST_STATE data, IN PMODULE module);
STATUS HandleMsrWrite(IN PCURRENT_GUEST_STATE data, IN PMODULE module);
STATUS HandleCpuId(IN PCURRENT_GUEST_STATE data, IN PMODULE module);
STATUS HandleEptViolation(IN PCURRENT_GUEST_STATE data, IN PMODULE module);
STATUS HandleInvalidGuestState(IN PCURRENT_GUEST_STATE data, IN PMODULE module);
STATUS HandleInvalidMsrLoading(IN PCURRENT_GUEST_STATE data, IN PMODULE module);
STATUS HandleMachineCheckFailure(IN PCURRENT_GUEST_STATE data, IN PMODULE module);
STATUS HandleCrAccess(IN PCURRENT_GUEST_STATE data, IN PMODULE module);
STATUS HandleTripleFault(IN PCURRENT_GUEST_STATE data, IN PMODULE module);
STATUS HandleApicInit(IN PCURRENT_GUEST_STATE data, IN PMODULE module);
STATUS HandleApicSipi(IN PCURRENT_GUEST_STATE data, IN PMODULE module);

#endif