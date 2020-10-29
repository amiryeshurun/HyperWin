SECTIONS
{
    /* Match the CODE_BEGIN_ADDRESS from entrypoint.asm */
    . = 0x3300000;
    /* Use all text sections from input files */
    .text : { *(.text) }
    .nt_data : 
    {
        __ntDataStart = .;
        *(.nt_data)
    }
    /* Use all data sections from input files */
    .data : { *(.data) } 
    /* Use all bss sections from input files */
    .bss : { *(.bss) }
    /* Use all rodata sections from input files */
    .rodata : { *(.rodata) }
    /DISCARD/ :
    {
        *(.eh_frame)
    }
}
