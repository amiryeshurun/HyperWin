# --------------- C COMPILER --------------- #
C_COMPILER 		 = gcc
C_COMPILER_FLAGS = -I./native-hypervisor/include \
				   -nostdlib \
				   -c \
				   -w \
				   -fno-stack-protector \
				   -mno-red-zone \
				   -nostdinc \
				   -g

# --------------- LINKER --------------- #
LINKER 			= ld
LINKER_FLAGS    = -nostdlib \
				  -T build/linker_script.ld \
				  --oformat elf64-x86-64 \
				  -n

# --------------- ASM COMPILER --------------- #
ASM_COMPILER 	= nasm
ASM_FLAGS 		= -I./native-hypervisor/include \
			      -f elf64 \
				  -w-all


OBJDIR := build
SRC_DIR := native-hypervisor

ENTRYPOINT_ASM		      = entrypoint.asm
BIOS_C_SOURCE_FILES       = $(addprefix bios/, $(shell find native-hypervisor/bios/ -maxdepth 1 -name '*.c' -printf '%f '))
DEBUG_C_SOURCE_FILES      = $(addprefix debug/, $(shell find native-hypervisor/debug/ -maxdepth 1 -name '*.c' -printf '%f '))
UTILS_C_SOURCE_FILES      = $(addprefix utils/, $(shell find native-hypervisor/utils/ -maxdepth 1 -name '*.c' -printf '%f '))
VMM_C_SOURCE_FILES        = $(addprefix vmm/, $(shell find native-hypervisor/vmm/ -maxdepth 1 -name '*.c' -printf '%f '))
WIN_KERNEL_C_SOURCE_FILES = $(addprefix win_kernel/, $(shell find native-hypervisor/win_kernel/ -maxdepth 1 -name '*.c' -printf '%f '))
BIOS_ASM_SOURCE_FILES     = $(addprefix bios/, $(shell find native-hypervisor/bios/ -maxdepth 1 -name '*.asm' -printf '%f '))
UTILS_ASM_SOURCE_FILES    = $(addprefix utils/, $(shell find native-hypervisor/utils/ -maxdepth 1 -name '*.asm' -printf '%f '))
VMM_ASM_SOURCE_FILES      = $(addprefix vmm/, $(shell find native-hypervisor/vmm/ -maxdepth 1 -name '*.asm' -printf '%f '))

OUTPUT_OBJECT_FILES = $(addprefix $(OBJDIR)/, $(ENTRYPOINT_ASM:.asm=.o))           \
					  $(addprefix $(OBJDIR)/, $(BIOS_ASM_SOURCE_FILES:.asm=.o))	   \
					  $(addprefix $(OBJDIR)/, $(UTILS_ASM_SOURCE_FILES:.asm=.o))   \
					  $(addprefix $(OBJDIR)/, $(VMM_ASM_SOURCE_FILES:.asm=.o))     \
					  $(addprefix $(OBJDIR)/, $(BIOS_C_SOURCE_FILES:.c=.o))  	   \
					  $(addprefix $(OBJDIR)/, $(DEBUG_C_SOURCE_FILES:.c=.o)) 	   \
					  $(addprefix $(OBJDIR)/, $(UTILS_C_SOURCE_FILES:.c=.o))       \
					  $(addprefix $(OBJDIR)/, $(VMM_C_SOURCE_FILES:.c=.o))         \
					  $(addprefix $(OBJDIR)/, $(WIN_KERNEL_C_SOURCE_FILES:.c=.o))

.PHONY: clean

all: clean \
	$(OBJDIR)/hypervisor.iso

$(OBJDIR)/%.o : $(SRC_DIR)/%.c
	$(C_COMPILER) $(C_COMPILER_FLAGS) $< -o $@

$(OBJDIR)/%.o : $(SRC_DIR)/%.asm
	$(ASM_COMPILER) $(ASM_FLAGS) $< -o $@

$(OBJDIR)/hypervisor.so : $(OUTPUT_OBJECT_FILES)
	$(LINKER) $(LINKER_FLAGS) $(OUTPUT_OBJECT_FILES) -o $@

$(OBJDIR)/hypervisor.iso : $(OBJDIR)/hypervisor.so
	cp $< $(OBJDIR)/iso/boot/$(notdir $<)
	cd $(OBJDIR) && grub-mkrescue -o $(notdir $@) iso

clean:
	@rm -f $(OUTPUT_OBJECT_FILES)
	@rm -f $(OBJDIR)/hypervisor.so
	@rm -f $(OBJDIR)/iso/boot/hypervisor.so
	@rm -f $(OBJDIR)/hypervisor.iso