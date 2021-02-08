#ifndef __VMEXIT_HANDLERS_H_
#define __VMEXIT_HANDLERS_H_

#include <vmm/vmm.h>
#include <vmx_modules/module.h>

STATUS DfltModuleInitializeAllCores(PMODULE module);
/* VM-Exit handlers */
STATUS DfltEmulateXSETBV(IN PCURRENT_GUEST_STATE data, IN PMODULE module);
STATUS DfltHandleVmCall(IN PCURRENT_GUEST_STATE data, IN PMODULE module);
STATUS DfltHandleMsrRead(IN PCURRENT_GUEST_STATE data, IN PMODULE module);
STATUS DfltHandleMsrWrite(IN PCURRENT_GUEST_STATE data, IN PMODULE module);
STATUS DfltHandleCpuId(IN PCURRENT_GUEST_STATE data, IN PMODULE module);
STATUS DfltHandleEptViolation(IN PCURRENT_GUEST_STATE data, IN PMODULE module);
STATUS DfltHandleInvalidGuestState(IN PCURRENT_GUEST_STATE data, IN PMODULE module);
STATUS DfltHandleInvalidMsrLoading(IN PCURRENT_GUEST_STATE data, IN PMODULE module);
STATUS DfltHandleMachineCheckFailure(IN PCURRENT_GUEST_STATE data, IN PMODULE module);
STATUS DfltHandleCrAccess(IN PCURRENT_GUEST_STATE data, IN PMODULE module);
STATUS DfltHandleTripleFault(IN PCURRENT_GUEST_STATE data, IN PMODULE module);
STATUS DfltHandleApicInit(IN PCURRENT_GUEST_STATE data, IN PMODULE module);
STATUS DfltHandleApicSipi(IN PCURRENT_GUEST_STATE data, IN PMODULE module);
STATUS DfltHandleException(IN PCURRENT_GUEST_STATE data, IN PMODULE module);

#endif