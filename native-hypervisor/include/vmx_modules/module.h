#ifndef __MODULE_H_
#define __MODULE_H_

#include <types.h>

#define VMEXIT_HANDLERS_MAX 100

struct _CURRENT_GUEST_STATE;
struct _SHARED_CPU_DATA;

typedef STATUS (*VMEXIT_HANDLER)(struct _CURRENT_GUEST_STATE*);

typedef struct _MODULE
{
    PCHAR moduleName;
    BOOL isHandledOnVmExit[VMEXIT_HANDLERS_MAX];
    VMEXIT_HANDLER vmExitHandlers[VMEXIT_HANDLERS_MAX];
} MODULE, *PMODULE;

VOID RegisterVmExitHandler(IN PMODULE module, IN QWORD exitReason, IN VMEXIT_HANDLER handler);
VOID RegisterModule(IN struct _SHARED_CPU_DATA* sharedData, IN PMODULE module);
VOID InitModule(IN PMODULE module);

#endif