# --------------- C COMPILER --------------- #
C_COMPILER 		 = gcc
C_COMPILER_FLAGS = -I./include \
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
ASM_FLAGS 		= -Iinclude \
			      -f elf64 \
				  -w-all


OBJDIR := build

GUEST_C_SOURCE_FILES  = $(addprefix guest/, $(shell find guest/ -maxdepth 1 -name '*.c' -printf '%f '))
HOST_C_SOURCE_FILES   = $(addprefix host/, $(shell find host/ -maxdepth 1 -name '*.c' -printf '%f '))
SHARED_C_SOURCE_FILES   = $(addprefix shared/, $(shell find shared/ -maxdepth 1 -name '*.c' -printf '%f '))
GUEST_ASM_SOURCE_FILES = $(addprefix guest/, $(shell find guest/ -maxdepth 1 -name '*.asm' -printf '%f '))
HOST_ASM_SOURCE_FILES   = $(addprefix host/, $(shell find host/ -maxdepth 1 -name '*.asm' -printf '%f '))
SHARED_ASM_SOURCE_FILES   = $(addprefix shared/, $(shell find shared/ -maxdepth 1 -name '*.asm' -printf '%f '))

OUTPUT_OBJECT_FILES = $(addprefix $(OBJDIR)/, $(HOST_ASM_SOURCE_FILES:.asm=.o))	   \
					  $(addprefix $(OBJDIR)/, $(GUEST_ASM_SOURCE_FILES:.asm=.o))   \
					  $(addprefix $(OBJDIR)/, $(SHARED_ASM_SOURCE_FILES:.asm=.o))  \
					  $(addprefix $(OBJDIR)/, $(GUEST_C_SOURCE_FILES:.c=.o)) 	   \
					  $(addprefix $(OBJDIR)/, $(HOST_C_SOURCE_FILES:.c=.o)) 	   \
					  $(addprefix $(OBJDIR)/, $(SHARED_C_SOURCE_FILES:.c=.o)) 	   

.PHONY: clean

all: clean \
	$(OBJDIR)/hypervisor.iso

$(OBJDIR)/%.o : %.c
	$(C_COMPILER) $(C_COMPILER_FLAGS) $< -o $@

$(OBJDIR)/%.o : %.asm
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