#include <vmx_modules/default_module.h>
#include <debug.h>
#include <intrinsics.h>
#include <vmm/msr.h>
#include <vmm/memory_manager.h>
#include <vmm/vm_operations.h>
#include <vmm/vmcs.h>
#include <vmm/control_fields.h>
#include <x86_64.h>
#include <vmm/exit_reasons.h>
#include <win_kernel/memory_manager.h>
#include <bios/apic.h>
#include <guest_communication/vmcall_values.h>
#include <guest_communication/vmcall_handlers.h>

STATUS HandleCrAccess(IN PCURRENT_GUEST_STATE data, IN PMODULE module)
{
    // Disable CR3 access after the first time
    __vmwrite(CPU_BASED_VM_EXEC_CONTROL, vmread(CPU_BASED_VM_EXEC_CONTROL)
         & ~(CPU_BASED_CR3_LOAD_EXITING) & ~(CPU_BASED_CR3_STORE_EXITING));
    QWORD accessInformation = vmread(EXIT_QUALIFICATION), 
        operation = accessInformation & CR_ACCESS_TYPE_MASK;
    PREGISTERS regs = &(data->guestRegisters);
    switch(accessInformation & CR_ACCESS_CR_NUMBER_MASK)
    {
        case 0:
        {
            if(operation == CR_ACCESS_TYPE_LMSW)
                PrintDebugLevelDebug("An attempt to execute LMSW detected\n");
            else if(operation == CR_ACCESS_TYPE_CLTS)
                PrintDebugLevelDebug("An attempt to execute CLTS detected\n");
            else
                Print("An unknown instruction causing a CR ACCESS vm-exit detected\n");
            ASSERT(FALSE);
        }
        case 3: // mov to/from CR3
        {
            if(operation == CR_ACCESS_TYPE_MOV_TO_CR)
            {
                switch(accessInformation & CR_ACCESS_REGISTER_MASK)
                {
                    case CR_ACCESS_REGISTER_RAX:
                        __vmwrite(GUEST_CR3, regs->rax);
                        break;
                    case CR_ACCESS_REGISTER_RBX:
                        __vmwrite(GUEST_CR3, regs->rbx);
                        break;
                    case CR_ACCESS_REGISTER_RCX:
                        __vmwrite(GUEST_CR3, regs->rcx);
                        break;
                    case CR_ACCESS_REGISTER_RDX:
                        __vmwrite(GUEST_CR3, regs->rdx);
                        break;
                    case CR_ACCESS_REGISTER_RSP:
                        __vmwrite(GUEST_CR3, regs->rsp);
                        break;
                    case CR_ACCESS_REGISTER_RBP:
                        __vmwrite(GUEST_CR3, regs->rbp);
                        break;
                    case CR_ACCESS_REGISTER_RSI:
                        __vmwrite(GUEST_CR3, regs->rsi);
                        break;
                    case CR_ACCESS_REGISTER_RDI:
                        __vmwrite(GUEST_CR3, regs->rdi);
                        break;
                    case CR_ACCESS_REGISTER_R8:
                        __vmwrite(GUEST_CR3, regs->r8);
                        break;
                    case CR_ACCESS_REGISTER_R9:
                        __vmwrite(GUEST_CR3, regs->r9);
                        break;
                    case CR_ACCESS_REGISTER_R10:
                        __vmwrite(GUEST_CR3, regs->r10);
                        break;
                    case CR_ACCESS_REGISTER_R11:
                        __vmwrite(GUEST_CR3, regs->r11);
                        break;
                    case CR_ACCESS_REGISTER_R12:
                        __vmwrite(GUEST_CR3, regs->r12);
                        break;
                    case CR_ACCESS_REGISTER_R13:
                        __vmwrite(GUEST_CR3, regs->r13);
                        break;
                    case CR_ACCESS_REGISTER_R14:
                        __vmwrite(GUEST_CR3, regs->r14);
                        break;
                    case CR_ACCESS_REGISTER_R15:
                        __vmwrite(GUEST_CR3, regs->r15);
                        break;
                    default:
                        Print("Could not find dest operand for CR ACCESS instruction\n");
                        return STATUS_UNKNOWN_OPERAND;
                }
            }
            else if(operation == CR_ACCESS_TYPE_MOV_FROM_CR)
            {
                QWORD cr3Value = vmread(GUEST_CR3);
                switch(accessInformation & CR_ACCESS_REGISTER_MASK)
                {
                    case CR_ACCESS_REGISTER_RAX:
                        regs->rax = cr3Value;
                        break;
                    case CR_ACCESS_REGISTER_RBX:
                        regs->rbx = cr3Value;
                        break;
                    case CR_ACCESS_REGISTER_RCX:
                        regs->rcx = cr3Value;
                        break;
                    case CR_ACCESS_REGISTER_RDX:
                        regs->rdx = cr3Value;
                        break;
                    case CR_ACCESS_REGISTER_RSP:
                        regs->rsp = cr3Value;
                        break;
                    case CR_ACCESS_REGISTER_RBP:
                        regs->rbp = cr3Value;
                        break;
                    case CR_ACCESS_REGISTER_RSI:
                        regs->rsi = cr3Value;
                        break;
                    case CR_ACCESS_REGISTER_RDI:
                        regs->rdi = cr3Value;
                        break;
                    case CR_ACCESS_REGISTER_R8:
                        regs->r8 = cr3Value;
                        break;
                    case CR_ACCESS_REGISTER_R9:
                        regs->r9 = cr3Value;
                        break;
                    case CR_ACCESS_REGISTER_R10:
                        regs->r10 = cr3Value;
                        break;
                    case CR_ACCESS_REGISTER_R11:
                        regs->r11 = cr3Value;
                        break;
                    case CR_ACCESS_REGISTER_R12:
                        regs->r12 = cr3Value;
                        break;
                    case CR_ACCESS_REGISTER_R13:
                        regs->r13 = cr3Value;
                        break;
                    case CR_ACCESS_REGISTER_R14:
                        regs->r14 = cr3Value;
                        break;
                    case CR_ACCESS_REGISTER_R15:
                        regs->r15 = cr3Value;
                        break;
                    default:
                        Print("Could not find dest operand for CR ACCESS instruction\n");
                        return STATUS_UNKNOWN_OPERAND;
                }
            }
        }
    }
    regs->rip += vmread(VM_EXIT_INSTRUCTION_LEN);
    return STATUS_SUCCESS;
}

STATUS EmulateXSETBV(IN PCURRENT_GUEST_STATE data, IN PMODULE module)
{
#ifdef DEBUG_XSETBV
    PrintDebugLevelDebug("XSETBV detected, emulating the instruction.\n");
#endif
    // XSETBV ---> XCR[ECX] = EDX:EAX
    PREGISTERS regs = &(data->guestRegisters);
    QWORD eax, ebx, ecx, edx;
    __cpuid(1, 0, &eax, &ebx, &ecx, &edx);

    if((regs->rcx & 0xffffffffULL)
        || !(regs->rax & 1)
        || ((regs->rax & (1 << 2)) && !(regs->rax & (1 << 1)))
        || !(vmread(GUEST_CR4) & CR4_OSXSAVE)
        || !(ecx & CPUID_XSAVE)
        )
    {
        PrintDebugLevelDebug("--- Emulation was cancelled\n"); 
        return STATUS_SUCCESS;
    }
    __xsetbv(regs->rdx, regs->rax, regs->rcx);
    regs->rip += vmread(VM_EXIT_INSTRUCTION_LEN);
    return STATUS_SUCCESS;
}

STATUS HandleVmCall(IN PCURRENT_GUEST_STATE data, IN PMODULE module)
{    
    PREGISTERS regs = &(data->guestRegisters);

    if(regs->rax == VMCALL_SETUP_BASE_PROTECTION)
    {
        ASSERT(SetupHypervisorCodeProtection(data->currentCPU->sharedData, 
            data->currentCPU->sharedData->physicalCodeBase, 
            data->currentCPU->sharedData->codeBaseSize) == STATUS_SUCCESS);
        regs->rip = MBR_ADDRESS;
        return STATUS_SUCCESS;
    }
    else if(regs->rip == E820_VMCALL_GATE)
    {
        if(regs->rax == 0xE820)
        {
            BOOL carrySet = FALSE;
            if(regs->rbx >= data->currentCPU->sharedData->memoryRangesCount)
            {
                carrySet = TRUE;
                goto EmulateIRET;
            }
            regs->rax = E820_MAGIC;
            regs->rcx = (regs->rcx & ~(0xffULL)) | 20;
            CopyMemory(vmread(GUEST_ES_BASE) + (regs->rdi & 0xffffULL), 
                &(data->currentCPU->sharedData->allRam[regs->rbx++]), sizeof(E820_LIST_ENTRY));
            carrySet = FALSE;
            if(regs->rbx == data->currentCPU->sharedData->memoryRangesCount)
                regs->rbx = 0;
EmulateIRET:
            regs->rip = (DWORD)(*(DWORD_PTR)(vmread(GUEST_SS_BASE) + regs->rsp));
            WORD csValue = *(WORD_PTR)(vmread(GUEST_SS_BASE) + regs->rsp + 2);
            WORD flags = *(WORD_PTR)(vmread(GUEST_SS_BASE) + regs->rsp + 4);
            if(carrySet) 
                flags |= RFLAGS_CARRY;
            else
                flags &= ~(RFLAGS_CARRY);
            __vmwrite(GUEST_CS_BASE, csValue << 4);
            __vmwrite(GUEST_CS_SELECTOR, csValue);
            regs->rsp += 6;
            return STATUS_SUCCESS;
        }
        regs->rip = data->currentCPU->sharedData->int15Offset;
        __vmwrite(GUEST_CS_BASE, (data->currentCPU->sharedData->int15Segment));
        __vmwrite(GUEST_CS_SELECTOR, (data->currentCPU->sharedData->int15Segment) >> 4);
        return STATUS_SUCCESS;
    }
    else if(regs->rax == VMCALL_COMMUNICATION_BLOCK)
    {
        regs->rip += vmread(VM_EXIT_INSTRUCTION_LEN);
        return HandleVmCallCommunication(data);
    } 

    return STATUS_UNKNOWN_VMCALL;
}

STATUS HandleMsrRead(IN PCURRENT_GUEST_STATE data, IN PMODULE module)
{
	PREGISTERS regs = &(data->guestRegisters);
	regs->rip += vmread(VM_EXIT_INSTRUCTION_LEN);

	QWORD msrNum = regs->rcx & 0xFFFFFFFFULL;
	QWORD msrValue = __readmsr(msrNum);

	regs->rax = (DWORD)msrValue;
	regs->rdx = (DWORD)(msrValue >> 32);
    
    return STATUS_SUCCESS;
}

STATUS HandleMsrWrite(IN PCURRENT_GUEST_STATE data, IN PMODULE module)
{
	PREGISTERS regs = &(data->guestRegisters);
	regs->rip += vmread(VM_EXIT_INSTRUCTION_LEN);

	QWORD msrNum = regs->rcx & 0xFFFFFFFFULL;
	QWORD msrValue = (regs->rax & 0xFFFFFFFFULL) | ((regs->rdx & 0xFFFFFFFFULL) << 32);
	__writemsr(msrNum, msrValue);

    return STATUS_SUCCESS;
}

STATUS HandleCpuId(IN PCURRENT_GUEST_STATE data, IN PMODULE module)
{
    PREGISTERS regs = &(data->guestRegisters);
    QWORD eax, ebx, ecx, edx, leaf = regs->rax, subleaf = regs->rcx;
    regs->rip += vmread(VM_EXIT_INSTRUCTION_LEN);
    if(leaf == CPUID_GET_READ_PIPE)
    {
        QWORD physicalCommunication = data->currentCPU->sharedData->readPipe.physicalAddress;
        Print("Received a request for read pipe base address: %8\n", physicalCommunication);
        regs->rdx = physicalCommunication >> 32;
        regs->rax = physicalCommunication & 0xffffffffULL;

        return STATUS_SUCCESS;
    }
    else if(leaf == CPUID_GET_WRITE_PIPE)
    {
        QWORD physicalCommunication = data->currentCPU->sharedData->writePipe.physicalAddress;
        Print("Received a request for write pipe base address: %8\n", physicalCommunication);
        regs->rdx = physicalCommunication >> 32;
        regs->rax = physicalCommunication & 0xffffffffULL;

        return STATUS_SUCCESS;
    }
    __cpuid(leaf, subleaf, &eax, &ebx, &ecx, &edx);
    if (leaf == 1)
    {
        // According to Xen, this is the right way to handle XSAVE availability
	    if (vmread(GUEST_CR4) & CR4_OSXSAVE)
	        ecx |= (1 << CPUID_OSXSAVE);
        else
            ecx &= ~(1 << CPUID_OSXSAVE);
    }
    else if (leaf == CPUID_XSTATE_LEAF)
    {
        if(subleaf == 1)
        {
            /* 
                Bit 1: Supports XSAVEC and the compacted form of XRSTOR if set. -> V
                Bit 02: Supports XGETBV with ECX = 1 if set.                    -> X
                Bit 03: Supports XSAVES/XRSTORS and IA32_XSS if set.            -> X

                The rest are reserved. 
            */
            eax = 1;
            ebx = 0;
            ecx = 0;
            edx = 0;
        }
    }
    regs->rax = eax;
    regs->rbx = ebx;
    regs->rcx = ecx;
    regs->rdx = edx;    
    return STATUS_SUCCESS;
}

STATUS HandleEptViolation(IN PCURRENT_GUEST_STATE data, IN PMODULE module)
{
    if(CheckAccessToHiddenBase(data->currentCPU->sharedData, vmread(GUEST_PHYSICAL_ADDRESS)))
    {
        Print("!!! DETECTED ACCESS TO HYPERVISOR AREA !!!\n");
        return STATUS_ACCESS_TO_HIDDEN_BASE;
    }
    Print("Unhandled EPT Violation occured at: (P)%8, (RIP)%8\n", 
        vmread(GUEST_PHYSICAL_ADDRESS), vmread(GUEST_CS_BASE) + data->guestRegisters.rip);
    return STATUS_UNHANDLED_EPT_VIOLATION;
}

STATUS HandleInvalidGuestState(IN PCURRENT_GUEST_STATE data, IN PMODULE module)
{
    Print("Invalid Guest State!\n");
    return STATUS_INVALID_GUEST_STATE;
}

STATUS HandleInvalidMsrLoading(IN PCURRENT_GUEST_STATE data, IN PMODULE module)
{
    Print("INVALID MSR LOADING!\n");
    return STATUS_INVALID_MSR_LOADING;
}

STATUS HandleMachineCheckFailure(IN PCURRENT_GUEST_STATE data, IN PMODULE module)
{
    Print("Failure due to machine-check event!\n");
    return STATUS_MACHINE_CHECK_FAILURE;
}

STATUS HandleTripleFault(IN PCURRENT_GUEST_STATE data, IN PMODULE module)
{
    Print("!!! TRIPLE FAULT !!!\n");
    return STATUS_TRIPLE_FAULT;
}

STATUS HandleApicInit(IN PCURRENT_GUEST_STATE data, IN PMODULE module)
{
    // Intel SDM, Volume 3C, Section 33.5
    Print("INIT interrupt detected on core %d\n", data->currentCPU->coreIdentifier);
    __vmwrite(GUEST_ACTIVITY_STATE, CPU_STATE_WAIT_FOR_SIPI);    
    return STATUS_SUCCESS;
}

STATUS HandleApicSipi(IN PCURRENT_GUEST_STATE data, IN PMODULE module)
{
    QWORD vector = vmread(EXIT_QUALIFICATION);
    // See Intel SDM, Volume 3C, Section 25.2
    Print("SIPI detected on core: %d. Page number: %d\n", data->currentCPU->coreIdentifier, vector);
    PREGISTERS regs = &(data->guestRegisters);
    SetMemory(regs, 0, sizeof(REGISTERS));
    // PM & PG must be disabled
    __vmwrite(GUEST_CR0, (__readmsr(MSR_IA32_VMX_CR0_FIXED0) & (~CR0_PM_ENABLED) 
        & (~CR0_PG_ENABLED)) & __readmsr(MSR_IA32_VMX_CR0_FIXED1));
    __vmwrite(GUEST_EFER, 0); // Disable long mode
    __vmwrite(VM_ENTRY_CONTROLS, vmread(VM_ENTRY_CONTROLS) & (~VM_ENTRY_IA32E_MODE)); // ^^
    __vmwrite(GUEST_CR4, (__readmsr(MSR_IA32_VMX_CR4_FIXED0) | CR4_VMX_ENABLED) 
        & __readmsr(MSR_IA32_VMX_CR4_FIXED1));
    __vmwrite(GUEST_CR3, 0);
    // 0xffffffff limit causes an invalid guest state
    // Change CS to point to the SIPI page (muliply exit qualification by 0x1000)
    __vmwrite(GUEST_CS_SELECTOR, vector << 8);
    __vmwrite(GUEST_CS_BASE, vector << 12);
    __vmwrite(GUEST_CS_LIMIT, 0xffff);
    __vmwrite(GUEST_CS_AR_BYTES, vmread(GUEST_CS_AR_BYTES) & 0xff);
    __vmwrite(GUEST_ES_SELECTOR, 0);
    __vmwrite(GUEST_ES_BASE, 0);
    __vmwrite(GUEST_ES_LIMIT, 0xffff);
    __vmwrite(GUEST_ES_AR_BYTES, vmread(GUEST_ES_AR_BYTES) & 0xff);
    __vmwrite(GUEST_SS_SELECTOR, 0);
    __vmwrite(GUEST_SS_BASE, 0);
    __vmwrite(GUEST_SS_LIMIT, 0xffff);
    __vmwrite(GUEST_SS_AR_BYTES, vmread(GUEST_SS_AR_BYTES) & 0xff);
    __vmwrite(GUEST_DS_SELECTOR, 0);
    __vmwrite(GUEST_DS_BASE, 0);
    __vmwrite(GUEST_DS_LIMIT, 0xffff);
    __vmwrite(GUEST_DS_AR_BYTES, vmread(GUEST_DS_AR_BYTES) & 0xff);
    __vmwrite(GUEST_FS_SELECTOR, 0);
    __vmwrite(GUEST_FS_BASE, 0);
    __vmwrite(GUEST_FS_LIMIT, 0xffff);
    __vmwrite(GUEST_FS_AR_BYTES, vmread(GUEST_FS_AR_BYTES) & 0xff);
    __vmwrite(GUEST_GS_SELECTOR, 0);
    __vmwrite(GUEST_GS_BASE, 0);
    __vmwrite(GUEST_GS_LIMIT, 0xffff);
    __vmwrite(GUEST_GS_AR_BYTES, vmread(GUEST_GS_AR_BYTES) & 0xff);
    __vmwrite(GUEST_TR_SELECTOR, 0);
    __vmwrite(GUEST_TR_BASE, 0);
    __vmwrite(GUEST_TR_LIMIT, 0xffff);
    __vmwrite(GUEST_TR_AR_BYTES, vmread(GUEST_TR_AR_BYTES) & 0xff);
    // mark core as active
    __vmwrite(GUEST_ACTIVITY_STATE, CPU_STATE_ACTIVE);
    return STATUS_SUCCESS;
}

STATUS HandleException(IN PCURRENT_GUEST_STATE data, IN PMODULE module)
{
    QWORD vector = vmread(VM_EXIT_INTR_INFO);
    if(vector & 0xff != INT_PAGE_FAULT)
        return STATUS_VM_EXIT_NOT_HANDLED;
    __writecr2(vmread(EXIT_QUALIFICATION));
    ASSERT(InjectGuestInterrupt(INT_PAGE_FAULT, vmread(VM_EXIT_INTR_ERROR_CODE)) == STATUS_SUCCESS);
    return STATUS_SUCCESS;
}