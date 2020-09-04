#include <types.h>
#include <guest/bios_os_loader.h>

VOID Initialize()
{
    LoadMBRToEntryPoint();
    CopyMemory((QWORD_PTR)REAL_MODE_CODE_START, (QWORD_PTR)SetupSystemAndHandleControlToBios,
        SetupSystemAndHandleControlToBiosEnd - SetupSystemAndHandleControlToBios);
}