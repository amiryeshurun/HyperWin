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

typedef struct _GENETIC_MODULE_DATA
{
    union
    {
        struct _HOOKING_MODULE_DATA
        {
            PMODULE kppModule;
        } hookingModule;
        struct _KPP_INIT_MODULE_DATA
        {
            PMODULE hookingModule;
        } kppModule;
    };
} GENERIC_MODULE_DATA, *PGENERIC_MODULE_DATA;

typedef STATUS (*MODULE_INITIALIZER)(struct _SHARED_CPU_DATA*, struct _MODULE*, IN PGENERIC_MODULE_DATA);

VOID MdlRegisterVmExitHandler(IN PMODULE module, IN QWORD exitReason, IN VMEXIT_HANDLER handler);
VOID MdlRegisterModule(IN struct _SHARED_CPU_DATA* sharedData, IN PMODULE module);
VOID MdlInitModule(IN struct _SHARED_CPU_DATA* sharedData, IN PMODULE module, IN MODULE_INITIALIZER moduleInitializer, 
    IN PGENERIC_MODULE_DATA moduleData, IN VMEXIT_HANDLER defaultHandler);
VOID MdlSetModuleName(IN struct _SHARED_CPU_DATA* sharedData, IN PMODULE module, IN PCHAR name);
STATUS MdlGetModuleByName(OUT PMODULE* module, IN PCHAR name);

#endif