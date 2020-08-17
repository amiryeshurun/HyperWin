# System Call Entry

On 32 bit Windows machines, the transition from user mode to kernel mode is done using the `sysenter` instruction.
This instruction executes a fast call to a level 0 system procedure or routine.

When the SYSENTER instruction is executed, the processor does the following:
* Loads the segment selector from the SYSENTER_CS_MSR into the CS register.
* Loads the instruction pointer from the SYSENTER_EIP_MSR into the EIP register.
* Adds 8 to the value in SYSENTER_CS_MSR and loads it into the SS register.
* Loads the stack pointer from the SYSENTER_ESP_MSR into the ESP register.

(credit: https://c9x.me/x86/html/file_module_x86_id_313.html)

In order to intercepte system calls, we can override the fields in the SSDT - a shared table in the kernel address space that contains the addresses for each system call hander. To understand how to do that, the most important thing is to find out how the OS is calling the handlers.

To do that, we check the MSR value of SYSENTER_EIP_MSR (using Kernel Debug). Then, we disassemble the handler function, called KiFastCallEntry. There are many comments next to each line, explaining what is the purpose of each instruction.
