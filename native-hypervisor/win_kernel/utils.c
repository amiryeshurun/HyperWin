#include <win_kernel/utils.h>
#include <vmm/vmm.h>
#include <win_kernel/memory_manager.h>
#include <vmx_modules/hooking_module.h>
#include <win_kernel/file.h>
#include <win_kernel/process.h>
#include <win_kernel/component.h>
#include <vmm/memory_manager.h>

static __attribute__((section(".nt_thread_events"))) THREAD_EVENT g_threadEvents[25000];

VOID WinGetParameters(OUT QWORD_PTR params, IN BYTE count)
{
    PREGISTERS regs = &VmmGetVmmStruct()->guestRegisters;
    QWORD paramsStart = regs->rsp + 5 * sizeof(QWORD);
    switch(count)
    {
        case 17:
        case 16:
        case 15:
        case 14:
        case 13:
        case 12:
        case 11:
        case 10:
        case 9:
        case 8:
        case 7:
        case 6:
        case 5:
            WinMmCopyGuestMemory(params + 4, paramsStart, (count - 4) * sizeof(QWORD));
        case 4:
            params[3] = regs->r9;
        case 3:
            params[2] = regs->r8;
        case 2:
            params[1] = regs->rdx;
        case 1:
            params[0] = regs->rcx;
    }
}

VOID WinHookReturnEvent(IN QWORD rsp, IN QWORD threadId, IN QWORD hookAddress)
{
    QWORD returnAddress;
    
    returnAddress = CALC_RETURN_HOOK_ADDR(hookAddress);
    WinMmCopyGuestMemory(&g_threadEvents[threadId].returnAddress, rsp, sizeof(QWORD));
    WinMmCopyMemoryToGuest(rsp, &returnAddress, sizeof(QWORD));
}

PTHREAD_EVENT WinGetEventForThread(IN QWORD threadId)
{
    return &(g_threadEvents[threadId]);
}

VOID WinInitializeComponents()
{
    BYTE_PTR componentsBegin, componentsEnd;
    QWORD componentsCount;
    PCOMPONENT_INIT_DATA currentComponent;

    componentsBegin = PhysicalToVirtual(&__components_config_segment);
    componentsEnd = PhysicalToVirtual(&__components_config_segment_end);
    componentsCount = (componentsEnd - componentsBegin) / sizeof(COMPONENT_INIT_DATA);

    for (QWORD i = 0; i < componentsCount; i++)
    {
        currentComponent = componentsBegin + i * sizeof(COMPONENT_INIT_DATA);
        currentComponent->initializer();
    }
}