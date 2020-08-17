# The Entrypoint

In order to make things simple, the hypervisor doesn't implement a bootloader. Instead, it uses GRUB (GNU's bootloader) with multiboot2. 

The first thing that we must do is to take care of multiboot2's headers, which are its configurations. Please visit [this site](https://www.gnu.org/software/grub/manual/multiboot2/multiboot.html) and start reading from section 3.1.1. I am not going to explain how the bootloader actually works.

When the instruction pointer reaches the actual entrypoint (called `_hypervisor_entrypoint`), it is in 32bit protected mode, with paging disabled. Hence, the first thing that we must take care of is setting up a temporary page table.  
Once we finish setting up the page table - we are ready to enter real mode!

Entering long mode is not complicated. You can reffer [to osdev wiki](https://wiki.osdev.org/Setting_Up_Long_Mode) to understand what is required.  When the CPU enters long mode, we are ready to call our C code, which will read Windows' MBR to `0x7c00` ([why?](https://www.glamenv-septzen.net/en/view/6)).