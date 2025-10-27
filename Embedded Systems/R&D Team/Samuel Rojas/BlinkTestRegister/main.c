#include <stdint.h>

/*COMPILING CMANDS:
 arm-none-eabi-gcc -o main.o -c -g -mcpu=cortex-m3 -mthumb main.c
 arm-none-eabi-gcc -o application.elf "-Wl,-Tlinker.ld" -nostartfiles main.o
 arm-none-eabi-objcopy -O binary application.elf application.bin
*/

#define RCC_BASE 0x40021000
/* We start at the Reset and Clock Control register and move to the 
* APB2 peripheral clock enable register, which enables the PORTC so we can
* configure our GPIO. IOPCEN (at x04) controls PORTC, if it is 0 I/O is disabled.
*/
#define RCC_APB2ENR_REGISTER (*(volatile uint32_t *)(RCC_BASE + 0x18))
#define RCC_APB2ENR_IOPCEN (1 << 4)

/* 0x4001100 corresponds to the memory group containing PC13 */
#define GPIO_PORTC_BASE 0x40011000
#define GPIO_CRH_REGISTER(x) (*(volatile uint32_t *)(x + 0x4))
#define GPIO_CHR_MODE_MASK(x) (0x3 << ((x - 8) * 4))
#define GPIO_CHR_MODE_OUTPUT(x) (0x1 << ((x - 8) * 4))
#define GPIO_BLINK_PORT GPIO_PORTC_BASE
#define GPIO_BLINK_NUM 13

#define GPIO_ODR_REGISTER(x) (*(volatile uint32_t *)(x + 0xC))
#define GPIO_ODR_PIN(x) (1 << x)

/*To compile: arm-none-eabi-gcc -o main.o -c -g -mcpu=cortex-m3 -mthumb main.c*/
int main(void) {
    // Enable port C clock gate.
    // Sets fifth bit to 1, enabling GPIO
    RCC_APB2ENR_REGISTER |= RCC_APB2ENR_IOPCEN;

    // Configure GPIO C pin 13 as output.
    GPIO_CRH_REGISTER(GPIO_BLINK_PORT) &= ~
    (GPIO_CHR_MODE_MASK(GPIO_BLINK_NUM));
    GPIO_CRH_REGISTER(GPIO_BLINK_PORT) |= 
    GPIO_CHR_MODE_OUTPUT(GPIO_BLINK_NUM);

    for (;;) {
        // Set the output bit.
        GPIO_ODR_REGISTER(GPIO_BLINK_PORT) |= GPIO_ODR_PIN(GPIO_BLINK_NUM);
        for (uint32_t i = 0; i < 400000; ++i) {
            __asm__ volatile("nop");
        }
        // Reset it again.
        GPIO_ODR_REGISTER(GPIO_BLINK_PORT) &= ~GPIO_ODR_PIN(GPIO_BLINK_NUM);
        for (uint32_t i = 0; i < 100000; ++i) {
            __asm__ volatile("nop");
        }
    }

    return 0;
}