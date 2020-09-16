# --------------- C COMPILER --------------- #
C_COMPILER 		 = gcc
C_COMPILER_FLAGS = -Iinclude \
				   -nostdlib \
				   -w \
				   -fno-stack-protector \
				   -mno-red-zone

# --------------- LINKER --------------- #
LINKER 			= ld
LINKER_FLAGS    = 
# --------------- ASM COMPILER --------------- #
ASM_COMPILER 	= nasm
ASM_FLAGS 		= -Iinclude \
			      -f elf64 \
				  -w-all

				   