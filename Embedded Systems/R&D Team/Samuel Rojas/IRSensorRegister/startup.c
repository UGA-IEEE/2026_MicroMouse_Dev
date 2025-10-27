// startup.c - STM32F103C8T6 Startup Code

#include <stdint.h>

// External symbols from linker script
extern uint32_t _etext;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;
extern uint32_t _estack;

// Main function
extern int main(void);

// Function pointer type for vector table
typedef void (*vector_table_entry_t)(void);

// Default handler
void Default_Handler(void) {
    while(1);
}

// Cortex-M3 core interrupt handlers
void Reset_Handler(void);
void NMI_Handler(void) __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void MemManage_Handler(void) __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler(void) __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void) __attribute__((weak, alias("Default_Handler")));

// STM32 peripheral interrupt handlers (add more as needed)
void WWDG_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void PVD_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TAMPER_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void RTC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void FLASH_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void RCC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI4_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));

// Vector table structure
typedef struct {
    uint32_t *stack_pointer;
    vector_table_entry_t reset_handler;
    vector_table_entry_t nmi_handler;
    vector_table_entry_t hardfault_handler;
    vector_table_entry_t memmanage_handler;
    vector_table_entry_t busfault_handler;
    vector_table_entry_t usagefault_handler;
    uint32_t reserved1[4];
    vector_table_entry_t svc_handler;
    vector_table_entry_t debugmon_handler;
    uint32_t reserved2;
    vector_table_entry_t pendsv_handler;
    vector_table_entry_t systick_handler;
    vector_table_entry_t irq_handlers[11];
} vector_table_t;

// Vector table
__attribute__((section(".isr_vector")))
const vector_table_t vector_table = {
    .stack_pointer = &_estack,
    .reset_handler = Reset_Handler,
    .nmi_handler = NMI_Handler,
    .hardfault_handler = HardFault_Handler,
    .memmanage_handler = MemManage_Handler,
    .busfault_handler = BusFault_Handler,
    .usagefault_handler = UsageFault_Handler,
    .reserved1 = {0, 0, 0, 0},
    .svc_handler = SVC_Handler,
    .debugmon_handler = DebugMon_Handler,
    .reserved2 = 0,
    .pendsv_handler = PendSV_Handler,
    .systick_handler = SysTick_Handler,
    .irq_handlers = {
        WWDG_IRQHandler,
        PVD_IRQHandler,
        TAMPER_IRQHandler,
        RTC_IRQHandler,
        FLASH_IRQHandler,
        RCC_IRQHandler,
        EXTI0_IRQHandler,
        EXTI1_IRQHandler,
        EXTI2_IRQHandler,
        EXTI3_IRQHandler,
        EXTI4_IRQHandler
    }
};

// Reset handler - entry point
void Reset_Handler(void) {
    // Copy .data section from Flash to RAM
    uint32_t *src = &_etext;
    uint32_t *dst = &_sdata;
    while(dst < &_edata) {
        *dst++ = *src++;
    }
    
    // Zero out .bss section
    dst = &_sbss;
    while(dst < &_ebss) {
        *dst++ = 0;
    }
    
    // Call main
    main();
    
    // Hang if main returns
    while(1);
}