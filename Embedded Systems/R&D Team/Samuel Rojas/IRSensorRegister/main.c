// main.c - STM32F103C8T6 Blue Pill LED Blink
// LEDs on PB3, PB4, PB5

#include <stdint.h>

// RCC Register Addresses
#define RCC_BASE            0x40021000
#define RCC_APB2ENR         (*(volatile uint32_t *)(RCC_BASE + 0x18))

// GPIO Port B Register Addresses
#define GPIOB_BASE          0x40010C00
#define GPIOB_CRL           (*(volatile uint32_t *)(GPIOB_BASE + 0x00))
#define GPIOB_CRH           (*(volatile uint32_t *)(GPIOB_BASE + 0x04))
#define GPIOB_ODR           (*(volatile uint32_t *)(GPIOB_BASE + 0x0C))

// Enable GPIOB clock (bit 3 in APB2ENR)
#define RCC_APB2ENR_IOPBEN  (1 << 3)

// Simple delay function
void delay(volatile uint32_t count) {
    while(count--) {
        __asm__("nop");
    }
}

void gpio_init(void) {
    // Enable GPIOB clock
    RCC_APB2ENR |= RCC_APB2ENR_IOPBEN;
    
    // Small delay after clock enable
    for(volatile int i = 0; i < 1000; i++);
    
    // Configure PB3, PB4, PB5 as output push-pull, 50MHz
    // Each pin uses 4 bits in CRL register
    // MODE[1:0] = 11 (50MHz output), CNF[1:0] = 00 (push-pull)
    // So we need 0b0011 = 0x3 for each pin
    
    // Read current value
    uint32_t temp = GPIOB_CRL;
    
    // Clear bits for PB3, PB4, PB5 (bits 12-23 in CRL)
    temp &= ~(0xFFF << 12);
    
    // Set PB3, PB4, PB5 as output push-pull 50MHz
    temp |= (0x3 << 12);  // PB3
    temp |= (0x3 << 16);  // PB4
    temp |= (0x3 << 20);  // PB5
    
    // Write back
    GPIOB_CRL = temp;
    
    // Ensure all pins start LOW
    GPIOB_ODR &= ~((1 << 3) | (1 << 4) | (1 << 5));
}

int main(void) {
    gpio_init();
    
    while(1) {
        // Turn on PB3, others off
        GPIOB_ODR = (1 << 3);
        delay(500000);
        
        // Turn on PB4, others off
        GPIOB_ODR = (1 << 4);
        delay(500000);
        
        // Turn on PB5, others off
        GPIOB_ODR = (1 << 5);
        delay(500000);
        
        // All off
        GPIOB_ODR = 0;
        delay(500000);
        
        // All on
        GPIOB_ODR = (1 << 3) | (1 << 4) | (1 << 5);
        delay(500000);
        
        // All off
        GPIOB_ODR = 0;
        delay(500000);
    }
    
    return 0;
}