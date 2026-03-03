.syntax unified
.cpu cortex-m3
.thumb
.thumb_func
.global _start
.global Reset_Handler
.global main


/* ----------------------------
 * Vector Table
 * ---------------------------- */
.section .isr_vector, "a", %progbits
.word 0x20005000          /* Initial stack pointer */
.word Reset_Handler       /* Reset handler */
.word 0                   /* NMI_Handler */
.word 0                   /* HardFault_Handler */

/* Symbols from linker script */
.extern __bss_start__
.extern __bss_end__


.text

Reset_Handler:
    /* Zero initialize .bss */
    ldr r0, =__bss_start__
    ldr r1, =__bss_end__
    movs r2, #0
1:  cmp r0, r1
    it lt
    strlt r2, [r0], #4
    blt 1b

    /* Jump to C main() */
    bl main

1:  b 1b                 

