// ============================================================================
// STM32F1 Bare-Metal ADC + LED Debug Example
// ============================================================================

// -----------------------------------------------------------------------------
// Base Addresses
// -----------------------------------------------------------------------------
#define PERIPH_BASE        0x40000000
#define APB2PERIPH_BASE   (PERIPH_BASE + 0x10000)
#define AHBPERIPH_BASE    (PERIPH_BASE + 0x20000)

// RCC
#define RCC_BASE          (AHBPERIPH_BASE + 0x1000)
#define RCC_APB2ENR       (*(volatile unsigned int*)(RCC_BASE + 0x18))

// GPIO
#define GPIOA_BASE        (APB2PERIPH_BASE + 0x0800)
#define GPIOC_BASE        (APB2PERIPH_BASE + 0x1000)

#define GPIOA_CRL         (*(volatile unsigned int*)(GPIOA_BASE + 0x00))
#define GPIOC_CRH         (*(volatile unsigned int*)(GPIOC_BASE + 0x04))
#define GPIOC_ODR         (*(volatile unsigned int*)(GPIOC_BASE + 0x0C))

// ADC
#define ADC1_BASE         (APB2PERIPH_BASE + 0x2400)
#define ADC1_SR           (*(volatile unsigned int*)(ADC1_BASE + 0x00))
#define ADC1_CR1          (*(volatile unsigned int*)(ADC1_BASE + 0x04))
#define ADC1_CR2          (*(volatile unsigned int*)(ADC1_BASE + 0x08))
#define ADC1_SMPR2        (*(volatile unsigned int*)(ADC1_BASE + 0x10))
#define ADC1_SQR1         (*(volatile unsigned int*)(ADC1_BASE + 0x2C))
#define ADC1_SQR3         (*(volatile unsigned int*)(ADC1_BASE + 0x34))
#define ADC1_DR           (*(volatile unsigned int*)(ADC1_BASE + 0x4C))

// -----------------------------------------------------------------------------
// Bit Definitions
// -----------------------------------------------------------------------------
#define RCC_IOPAEN        (1 << 2)   // GPIOA clock enable
#define RCC_IOPCEN        (1 << 4)   // GPIOC clock enable
#define RCC_ADC1EN        (1 << 9)   // ADC1 clock enable

// ADC_CR2 bits
#define ADC_CR2_ADON      (1 << 0)   // ADC enable
#define ADC_CR2_CONT      (1 << 1)   // Continuous conversion
#define ADC_CR2_CAL       (1 << 2)   // Start calibration
#define ADC_CR2_RSTCAL    (1 << 3)   // Reset calibration
#define ADC_CR2_SWSTART   (1 << 22)  // Start conversion (software)

// ADC_SR bits
#define ADC_SR_EOC        (1 << 1)   // End of conversion flag

// -----------------------------------------------------------------------------
// Simple Delay Function
// -----------------------------------------------------------------------------
void delay(volatile unsigned int d) {
    while (d--) {
        __asm__("nop");
    }
}

// -----------------------------------------------------------------------------
// Debug LED Blink (on PC13, active-low)
// -----------------------------------------------------------------------------
void blink_debug(volatile unsigned int times) {
    // Enable GPIOC clock
    RCC_APB2ENR |= RCC_IOPCEN;

    // Configure PC13 as output (MODE13 = 10, CNF13 = 00)
    GPIOC_CRH &= ~(0xF << 20);  // Clear bits for pin 13
    GPIOC_CRH |=  (0x2 << 20);  // MODE13 = 2 MHz output

    while (times--) {
        // LED ON (active-low)
        GPIOC_ODR &= ~(1 << 13);
        delay(800000);

        // LED OFF
        GPIOC_ODR |= (1 << 13);
        delay(800000);
    }
}

// -----------------------------------------------------------------------------
// Main Program
// -----------------------------------------------------------------------------
int main(void) {
    while (1) {
        // --- Enable Clocks ---
        RCC_APB2ENR |= RCC_ADC1EN;  // ADC1 clock
        RCC_APB2ENR |= RCC_IOPAEN;  // GPIOA clock

        // --- Configure PA0 as analog input ---
        GPIOA_CRL &= ~(0xF << 0);   // CNF0=00, MODE0=00 -> Analog

        // --- Power on ADC ---
        ADC1_CR2 |= ADC_CR2_ADON;
        delay(10000);

        // --- Reset calibration ---
        ADC1_CR2 |= ADC_CR2_RSTCAL;
        while (ADC1_CR2 & ADC_CR2_RSTCAL);
        blink_debug(1);

        delay(5000000);

        // --- Start calibration ---
        ADC1_CR2 |= ADC_CR2_CAL;
        while (ADC1_CR2 & ADC_CR2_CAL);
        blink_debug(2);

        delay(5000000);

        // --- Start conversion ---
        ADC1_CR2 |= ADC_CR2_CONT;   // Continuous mode
        ADC1_SQR3 = 0;              // Channel 0
        delay(5000000);

        // Software start
        ADC1_CR2 |= ADC_CR2_SWSTART;

        // --- Wait for EOC flag ---
        while (!(ADC1_SR & ADC_SR_EOC));
        blink_debug(3);

        // --- Read result ---
        unsigned int result = ADC1_DR & 0xFFFF;
        (void)result; // Prevent compiler warning
    }
}
