<p align="left">
<a href="https://ci.appveyor.com/project/amiryeshurun/hyperwin/branch/master"><img src="https://ci.appveyor.com/api/projects/status/github/amiryeshurun/hyperwin?svg=true" alt="Build"></a>
<a href="https://www.gnu.org/licenses/gpl-3.0"><img src="https://img.shields.io/badge/License-GPLv3-blue.svg" alt="License"></a>
</p>

# HyperWin

HyperWin is a native hypervisor designed for Windows (x64 only) running on Intel processors. The whole system consists three major components: a hypervisor, a driver and a command line application. Using the command-line application, you can send IOCTL codes to the communication driver that will eventually reach the hypervisor.
HyperWin provides many interesting features, including: 
- Creation of memory regions hidden from the operating system (using EPT and a hidden hook on E820)
- PatchGaurd bypassing
- Sensitive data protection via IRP hooking (User-Mode & Kernel-Mode) 
- Smart process management
- A (very!) generic hooking module

And much more!

### Compilation
HyperWin can be installed on any computer that is using an MBR disk. The compilation process is super easy:
```sh
$ cd HyperWin
$ make
```
After running the above commands, a new file called `hypervisor.iso` will be generated inside the `build` directory. Burn it to a USB stick, plug the stick to a computer and make sure to change the boot order from the BIOS menu (the USB stick must be choosed as the first option).
Once the computer boots to HyperWin, it will automatically load Windows after it finishes the initialization process.

License
---

**HyperWin** is licensed under the [GPL v3.0](LICENSE) license.
