# --------------- C COMPILER --------------- #
C_COMPILER 		 = gcc
C_COMPILER_FLAGS = -Iinclude \
				   -nostdlib \
				   -w \
				   -fno-stack-protector \
				   -mno-red-zone

# --------------- LINKER --------------- #
LINKER 			= ld
LINKER_FLAGS    = -nostdlib \
				  -T build/linker_script.ld

# --------------- ASM COMPILER --------------- #
ASM_COMPILER 	= nasm
ASM_FLAGS 		= -Iinclude \
			      -f elf64 \
				  -w-all

OUTPUT_OBJECT_FILES = 

all:
	@echo TEST: $(GUEST_CODE_FILES)

				   