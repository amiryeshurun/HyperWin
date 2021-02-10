#ifndef __COMPONENT_H_
#define __COMPONENT_H_

#include <types.h>

typedef VOID (*ComponentInitializer)();

typedef struct _COMPONENT_INIT_DATA
{
    ComponentInitializer initializer;
}  COMPONENT_INIT_DATA, *PCOMPONENT_INIT_DATA;
// __attribute__((__packed__))
#define REGISTER_COMPONENT(init, shortName) \
    static __attribute__((section(".components_config"))) COMPONENT_INIT_DATA shortName##Component = \
    { .initializer = init }

extern BYTE_PTR __components_config_segment;
extern BYTE_PTR __components_config_segment_end;

#endif