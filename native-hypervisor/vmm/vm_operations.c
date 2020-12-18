#include <vmm/vm_operations.h>
#include <intrinsics.h>

QWORD vmread(IN QWORD field)
{
    QWORD value;
    
    __vmread(field, &value);
    return value;
}