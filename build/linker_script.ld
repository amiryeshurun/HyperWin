SECTIONS
{
    /* Match the CODE_BEGIN_ADDRESS from entrypoint.asm */
    . = 0x120000;
    /DISCARD/ : { *(.eh_frame) }
    /* Use all text sections from input file */
    .text : { *(.text) }
    /* Use all data sections from input file */
    .data : { *(.data) } 
    /* Use all bss sections from input file */
    .bss : { *(.bss) } 
}
