#ifndef __TYPES_H_
#define __TYPES_H_

typedef unsigned int UINT32;
typedef unsigned long long UINT64;
typedef unsigned char BOOL;
typedef UINT32 PTR;
typedef unsigned long DWORD;

#define FALSE (0)
#define TRUE (!(FALSE))

#define OUT
#define IN

#ifndef NULL
#define NULL (void*)0
#endif

#endif