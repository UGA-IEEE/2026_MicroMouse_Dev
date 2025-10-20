#define PERIPH_BASE     0x40000000
#define APB2PERIPH_BASE (PERIPH_BASE + 0x10000)
#define AHBPERIPH_BASE  (PERIPH_BASE + 0x20000)

#define RCC_BASE        (AHBPERIPH_BASE + 0x1000)
#define GPIOC_BASE      (APB2PERIPH_BASE + 0x1000)

#define RCC_APB2ENR     (*(volatile unsigned int*)(RCC_BASE + 0x18))
#define GPIOC_CRH       (*(volatile unsigned int*)(GPIOC_BASE + 0x04))
#define GPIOC_ODR       (*(volatile unsigned int*)(GPIOC_BASE + 0x0C))

#define RCC_IOPCEN      (1 << 4)   // I/O port C clock enable

void delay(volatile unsigned int d) {
    while (d--) {
        __asm__("nop");
    }
}

int main(void) {
    // Enable GPIOC clock
    RCC_APB2ENR |= RCC_IOPCEN;

    // Configure PC13 as output (MODE13 = 10, CNF13 = 00)
    GPIOC_CRH &= ~(0xF << 20);     // Clear bits for pin 13
    GPIOC_CRH |=  (0x2 << 20);     // MODE13 = 2 MHz output

    while (1) {
        // LED ON (remember: active-low)
        GPIOC_ODR &= ~(1 << 13);
        delay(800000);

        // LED OFF
        GPIOC_ODR |= (1 << 13);
        delay(800000);
    }
}
