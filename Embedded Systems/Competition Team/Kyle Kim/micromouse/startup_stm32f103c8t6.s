.syntax unified
.cpu cortex-m3
.thumb

.global _start
.global Reset_Handler
.global main

/* Vector table */
.section .isr_vector, "a", %progbits
.word 0x20005000       /* initial stack pointer */
.word Reset_Handler    /* reset handler */
.word 0                /* NMI_Handler */
.word 0                /* HardFault_Handler */

/* Symbols from linker script */
.extern __bss_start__
.extern __bss_end__

.text
Reset_Handler:
    /* zero .bss */
    ldr r0, =__bss_start__
    ldr r1, =__bss_end__
    movs r2, #0
1:  cmp r0, r1
    it lt
    strlt r2, [r0], #4
    blt 1b

    bl main        /* call main() */

1:  b 1b          /* loop forever if main returns */
