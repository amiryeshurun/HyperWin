#include <vmm/vmm.h>
#include <vmx_modules/module.h>

VOID InitModule(IN PMODULE module)
{
    for(QWORD i = 0; i < VMEXIT_HANDLERS_MAX; module->isHandledOnVmExit[i] = FALSE,
        module->vmExitHandlers[i] = NULL, i++);
    module->moduleName = NULL;
}

VOID RegisterVmExitHandler(IN PMODULE module, IN QWORD exitReason, IN VMEXIT_HANDLER handler)
{
    module->isHandledOnVmExit[exitReason] = TRUE;
    module->vmExitHandlers[exitReason] = handler;
}

VOID RegisterModule(IN struct _SHARED_CPU_DATA* sharedData, IN PMODULE module)
{
    // PMODULE* newArr = 
}

