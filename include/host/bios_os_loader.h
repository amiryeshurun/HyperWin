#ifndef __BIOS_OS_LOADER_H_
#define __BIOS_OS_LOADER_H_

#include <types.h>

VOID ReadFirstSectorToRam(IN BYTE diskIndex, OUT QWORD_PTR address);
VOID LoadMBRToEntryPoint();

#endif