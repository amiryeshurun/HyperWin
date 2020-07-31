# Handle Translation

One of the goals of the hypervisor is intercepting system calls.
It is knows that Windows manages objects using Handles, which are basically abstractions for all of the resource types that the OS manages.

In order to understand the meaning of PROCESS handles, we need to translate each PROCESS handle to the appropriate EPROCESS structure. This translation uses a very important structure, called handle table. Every process has its ows handle table.
The disassemble of these Windows' kernel funtions allows writing some simple C functions, giving the Host the ability to translate each handle to the corresponding EPROCESS structure.