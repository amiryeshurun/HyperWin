#include <vmm/vmm.h>
#include <vmx_modules/module.h>
#include <debug.h>

VOID MdlInitModule(IN PMODULE module)
{
    for(QWORD i = 0; i < VMEXIT_HANDLERS_MAX; module->isHandledOnVmExit[i] = FALSE,
            module->vmExitHandlers[i] = NULL, i++);
    module->moduleName = NULL;
}

VOID MdlRegisterVmExitHandler(IN PMODULE module, IN QWORD exitReason, IN VMEXIT_HANDLER handler)
{
    module->isHandledOnVmExit[exitReason] = TRUE;
    module->vmExitHandlers[exitReason] = handler;
}

VOID MdlRegisterModule(IN PMODULE module)
{
    PMODULE* newArr;
    PSHARED_CPU_DATA sharedData;

    sharedData = VmmGetVmmStruct()->currentCPU->sharedData;
    sharedData->heap.allocate(&sharedData->heap, (++sharedData->modulesCount) * sizeof(PMODULE), &newArr);
    HwCopyMemory(newArr, sharedData->modules, (sharedData->modulesCount - 1) * sizeof(PMODULE));
    newArr[sharedData->modulesCount - 1] = module;
    if(sharedData->modules)
        sharedData->heap.deallocate(&sharedData->heap, sharedData->modules);
    sharedData->modules = newArr;
}

VOID MdlSetModuleName(IN PMODULE module, IN PCHAR moduleName)
{
    PHEAP heap;

    heap = &VmmGetVmmStruct()->currentCPU->sharedData->heap;
    heap->allocate(heap, (StringLength(moduleName) + 1) * sizeof(CHAR),
        &module->moduleName);
    HwCopyMemory(module->moduleName, moduleName, StringLength(moduleName) + 1);
}

STATUS MdlGetModuleByName(OUT PMODULE* module, IN PCHAR name)
{
    PSHARED_CPU_DATA shared;
    QWORD nameLength;

    shared = VmmGetVmmStruct()->currentCPU->sharedData;
    nameLength = StringLength(name);
    for(QWORD i = 0; i < shared->modulesCount; i++)
    {
        if(!HwCompareMemory(name, shared->modules[i]->moduleName, nameLength))
        {
            *module = shared->modules[i];
            return STATUS_SUCCESS;
        }
    }
    return STATUS_MODULE_NOT_FOUND;
}