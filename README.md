# Windows Hypervisor

The goal of this project is to create a "small" hypervisor for the Windows operating system on 64-bit. I am aiming to finish writing it in about 5 months, before I start serving in the army at Match 2021. Unfortunately, I will have enough time to work on it only at the beginning of October 2020.

This hypervisor is not written for any specific usage. My main motivation is to learn about the virtualization technology on x86-64 Intel processors, about Windows internals and BIOS programming.

## Documentation

I am trying to write documentation for most of the parts of the hypervisor.  

### The Guest

* [Memory Managment](docs/guest/memory_manager.md) - reffers to [include/guest/memory_manager.h](include/guest/memory_manager.h)

### The Host

* [The Hypervisor Entrypoint](docs/host/entrypoint.md) - reffers to [host/entrypoint.asm](host/entrypoint.asm)

### Windows

* [What? Why?](docs/ntoskrnl/general.md) 
* [Translating Handles to Objects](docs/ntoskrnl/handle_translation.md) - reffers to [ntoskrnl/Windows 7/handle_translation](toskrnl/Windows%207/handle_translation)
* [System Calls Entrypoint on 32-bit](docs/ntoskrnl/syscall_entry.md) - reffers to [ntoskrnl/Windows 7/syscall_entry](ntoskrnl/Windows%207/syscall_entry)
