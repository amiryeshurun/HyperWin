#ifndef __KERNEL_OBJECTS_H_
#define __KERNEL_OBJECTS_H_

#include <types.h>
#include <error_codes.h>

STATUS GetCurrent_ETHREAD(IN BYTE_PTR kprcb, OUT BYTE_PTR ethread);
STATUS ETHREAD_GetField(IN BYTE_PTR ethread, IN QWORD field, OUT PVOID value);