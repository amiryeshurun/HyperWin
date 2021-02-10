SECTIONS
{
    /* Match the CODE_BEGIN_ADDRESS from entrypoint.asm */
    . = 0x3300000;
    /* Use all text sections from input files */
    .text : { *(.text) }
    /* Use all data sections from input files */
    .data : { *(.data) } 
    /* Use all bss sections from input files */
    .bss : { *(.bss) }
    /* Use all rodata sections from input files */
    .rodata : { *(.rodata) }
    .nt_thread_events :
    {
        __nt_thread_events_segment = .;
        *(.nt_thread_events)
    }
    .hooking_config :
    {
        __hooking_config_segment = .;
        *(.hooking_config)
    }
    .modules_config :
    {
        __modules_config_segment = .;
        *(.modules_config)
        __modules_config_segment_end = .;
    }
    .components_config :
    {
        __components_config_segment = .;
        *(.components_config)
        __components_config_segment_end = .;
    }
    /DISCARD/ :
    {
        *(.eh_frame)
    }
}
