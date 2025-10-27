MEMORY
{
    FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 32K
    RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 10K
}
/* Points to last avaiable address in RAM */
_reset_stack_pointer = ORIGIN(RAM) + LENGTH(RAM);

/*Specify the binary layout*/
SECTIONS
{
    .text : {
        /* Set the initial address to 0 */
        . = 0;
        LONG(_reset_stack_pointer);
        LONG(main | 1);
        /* The whole interrupt table is 332 bytes long. Advance to that position. */
        . = 332;
        /* And here comes the rest of the code */
        *(.text*)
    } > FLASH /* Put this in the flash memory region */
}