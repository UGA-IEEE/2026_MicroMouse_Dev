//Stuff for STM32
#define PERIPH_BASE     0x40000000
#define APB2PERIPH_BASE (PERIPH_BASE + 0x10000)
#define AHBPERIPH_BASE  (PERIPH_BASE + 0x20000)

#define RCC_BASE        (AHBPERIPH_BASE + 0x1000)
#define GPIOC_BASE      (APB2PERIPH_BASE + 0x1000)

#define RCC_APB2ENR     (*(volatile unsigned int*)(RCC_BASE + 0x18))
#define GPIOC_CRH       (*(volatile unsigned int*)(GPIOC_BASE + 0x04))
#define GPIOA_CRL	(*(volatile unsigned int*)(0x40010800))
#define GPIOC_ODR       (*(volatile unsigned int*)(GPIOC_BASE + 0x0C))

#define RCC_IOPCEN      (1 << 4)   // I/O port C clock enable


//ADC Base
#define ADC1_BASE        (APB2PERIPH_BASE + 0x02400)
#define RCC_IOPAEN      (1 << 2)                                      // GPIOA clock enable
#define RCC_ADC1EN      (1 << 9)                                      // ADC1 clock enable

#define ADC1_SR         (*(volatile unsigned int*)(ADC1_BASE + 0x00)) // Status register
#define ADC1_CR1        (*(volatile unsigned int*)(ADC1_BASE + 0x04)) // Control register 1
#define ADC1_CR2        (*(volatile unsigned int*)(ADC1_BASE + 0x08)) // Control register 2
#define ADC1_SMPR2      (*(volatile unsigned int*)(ADC1_BASE + 0x10)) // Sample time register 2
#define ADC1_SQR1       (*(volatile unsigned int*)(ADC1_BASE + 0x2C)) // Regular sequence register 1
#define ADC1_SQR3       (*(volatile unsigned int*)(ADC1_BASE + 0x34)) // Regular sequence register 3
#define ADC1_DR         (*(volatile unsigned int*)(ADC1_BASE + 0x4C)) // Data register

// ADC_CR2 bits
#define ADC_CR2_ADON    (1 << 0)   				      // ADC enable
#define ADC_CR2_CONT    (1 << 1)                                      // Continuous conversion
#define ADC_CR2_CAL     (1 << 2)                                      // Start calibration
#define ADC_CR2_SWSTART (1 << 22)                                     // Start conversion (software)
#define ADC_CR2_RSTCAL  (1 << 3)                                      // Reset calibration

// ADC_SR bits
#define ADC_SR_EOC      (1 << 1)                                      // End of conversion flag



void delay(volatile unsigned int d) {
    while (d--) {
        __asm__("nop");
    }
}

void blink_debug(volatile unsigned int time) {
	// Enable GPIOC clock
    	RCC_APB2ENR |= RCC_IOPCEN;

    	// Configure PC13 as output (MODE13 = 10, CNF13 = 00)
	GPIOC_CRH &= ~(0xF << 20);     // Clear bits for pin 13
	GPIOC_CRH |=  (0x2 << 20);     // MODE13 = 2 MHz output
	
	while (time--) {
		// LED ON (remember: active-low)
        	GPIOC_ODR &= ~(1 << 13);
        	delay(800000); //800000 is normal

        	// LED OFF
        	GPIOC_ODR |= (1 << 13);
        	delay(800000);
	}
}


int main(void) {
 

    while (1) {
       	RCC_APB2ENR |= (1 << 9);
	RCC_APB2ENR |= (1 << 2);
	GPIOA_CRL &= ~(0xF << 0);
	ADC1_CR2 |= (1 << 0);
	delay(10000);
	ADC1_CR2 |= (1 << 3);
	while (ADC1_CR2 & (1 << 3));
	blink_debug(1);
	delay(5000000);	
	ADC1_CR2 |= (1 << 2);
	while (ADC1_CR2 & (1 << 2));
	blink_debug(2);
	delay(5000000);
	ADC1_CR2 |= (1 << 1);
	ADC1_SQR3 = 0;
	delay(5000000);
	while (!(ADC1_SR & 1));
	blink_debug(3);
	unsigned int result = ADC1_DR & 0xFFFF;
    }
}
