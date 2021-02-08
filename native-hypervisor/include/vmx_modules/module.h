#ifndef __MODULE_H_
#define __MODULE_H_

#include <types.h>
#include <error_codes.h>
#include <utils/map.h>

#define VMEXIT_HANDLERS_MAX 70

struct _CURRENT_GUEST_STATE;
struct _SHARED_CPU_DATA;

typedef STATUS (*VMEXIT_HANDLER)(IN struct _CURRENT_GUEST_STATE*, IN struct _MODULE*);

typedef struct _MODULE
{
    PCHAR moduleName;
    BOOL isHandledOnVmExit[VMEXIT_HANDLERS_MAX];
    VMEXIT_HANDLER vmExitHandlers[VMEXIT_HANDLERS_MAX];
    PVOID moduleExtension;
    BOOL hasDefaultHandler;
    VMEXIT_HANDLER defaultHandler;
} MODULE, *PMODULE;

typedef STATUS (*MODULE_INITIALIZER)(IN PMODULE);
typedef STATUS (*SINGLE_CORE_INITIALIZER)(IN PMODULE);

#define MODULE_NAME_MAX_COUNT 50

typedef struct _MODULE_INIT_DATA
{
    MODULE_INITIALIZER globalInit;
    SINGLE_CORE_INITIALIZER singleInit;
} __attribute__((__packed__)) MODULE_INIT_DATA, *PMODULE_INIT_DATA;

#define REGISTER_MODULE(glob, single, shortName) \
    static __attribute__((section(".modules_config"))) MODULE_INIT_DATA shortName##Module = \
    { .globalInit = glob, .singleInit = single };

VOID MdlRegisterVmExitHandler(IN PMODULE module, IN QWORD exitReason, IN VMEXIT_HANDLER handler);
VOID MdlRegisterModule(IN PMODULE module);
VOID MdlInitModule(IN PMODULE module);
VOID MdlSetModuleName(IN PMODULE module, IN PCHAR name);
STATUS MdlGetModuleByName(OUT PMODULE* module, IN PCHAR name);

#endif