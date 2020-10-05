SECTIONS
{
    /* Match the CODE_BEGIN_ADDRESS from entrypoint.asm */
    . = 0x3300000;
    /* This section is used for exceptions handling in C++, but the HV is written in pure C */
    /DISCARD/ : { *(.eh_frame) }
    /* Use all text sections from input file */
    .text : { *(.text) }
    /* Use all data sections from input file */
    .data : { *(.data) } 
    /* Use all bss sections from input file */
    .bss : { *(.bss) } 
}
