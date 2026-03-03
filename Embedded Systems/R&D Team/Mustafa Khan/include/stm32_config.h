#ifndef STM32_CONFIG_H
#define STM32_CONFIG_H

#include "stm32f1xx.h"


#define MOTOR_L_PWM_PORT    GPIOA
#define MOTOR_L_PWM_PIN     GPIO_Pin_8      // PA8  - TIM1_CH1 - Left Motor A (IN_A_L) 
#define MOTOR_L_PWM_ALT     GPIO_Pin_9      // PA9  - TIM1_CH2 - Left Motor B (IN_B_L) 

#define MOTOR_R_PWM_PORT    GPIOA
#define MOTOR_R_PWM_PIN     GPIO_Pin_10     // PA10 - TIM1_CH3 - Right Motor B (IN_B_R) 
#define MOTOR_R_PWM_ALT     GPIO_Pin_11     // PA11 - TIM1_CH4 - Right Motor A (IN_A_R) 

#define PWM_FREQUENCY       20000           // PWM Frequency: 20 kHz 
#define PWM_RESOLUTION      1000            // PWM resolution: 0-1000 for duty cycle 

#define MOTOR_L_EN_B_PORT   GPIOB
#define MOTOR_L_EN_B_PIN    GPIO_Pin_5      // PB5 - Left Motor Enable B 

#define MOTOR_L_EN_A_PORT   GPIOB
#define MOTOR_L_EN_A_PIN    GPIO_Pin_6      // PB6 - Left Motor Enable A 

#define MOTOR_R_EN_B_PORT   GPIOB
#define MOTOR_R_EN_B_PIN    GPIO_Pin_3      // PB3 - Right Motor Enable B 

#define MOTOR_R_EN_A_PORT   GPIOB
#define MOTOR_R_EN_A_PIN    GPIO_Pin_4      // PB4 - Right Motor Enable A 

#define ENCODER_L_PORT      GPIOA
#define ENCODER_L_CH1_PIN   GPIO_Pin_0      // PA0  - TIM2_CH1 - Left Encoder A 
#define ENCODER_L_CH2_PIN   GPIO_Pin_1      // PA1  - TIM2_CH2 - Left Encoder B 

#define ENCODER_R_PORT      GPIOA
#define ENCODER_R_CH1_PIN   GPIO_Pin_2      // PA2  - TIM3_CH3 - Right Encoder A */
#define ENCODER_R_CH2_PIN   GPIO_Pin_3      // PA3  - TIM3_CH4 - Right Encoder B (ADC conflict) */

#define ENCODER_RESOLUTION  1024            /* IE2-1024: 1024 lines per rev (quadrature = 4096 counts) */
#define ENCODER_COUNTS_PER_REV (ENCODER_RESOLUTION * 4)  /* 4096 with quadrature encoding */

// Temp Wheel setup
#define WHEEL_DIAMETER_MM   32.0f           
#define WHEEL_CIRCUMFERENCE_MM (3.14159f * WHEEL_DIAMETER_MM)
#define WHEEL_BASE_MM       90.0f           
#define GEAR_RATIO          1.0f            

#define IMU_I2C_PORT        GPIOB
#define IMU_SCL_PIN         GPIO_Pin_8      /* PB8  - I2C1_SCL */
#define IMU_SDA_PIN         GPIO_Pin_9      /* PB9  - I2C1_SDA */
#define IMU_INT_PIN         GPIO_Pin_15     /* PB15 - H_INT */

#define IMU_BOOT_PIN        GPIO_Pin_14     /* PB14 - BOOT */
#define IMU_CLKSEL_PIN      GPIO_Pin_7      /* PC7  - CLKSEL0 */


#define DEBUG_USART         USART1
#define DEBUG_USART_PORT    GPIOA
#define DEBUG_USART_TX_PIN  GPIO_Pin_9      /* PA9  - USART1_TX (conflict with PWM) */
#define DEBUG_USART_RX_PIN  GPIO_Pin_10     /* PA10 - USART1_RX (conflict with PWM) */


#define SYSTEM_CLOCK_FREQ   72000000U       /* 72 MHz */
#define APB1_CLOCK_FREQ     36000000U       /* 36 MHz */
#define APB2_CLOCK_FREQ     72000000U       /* 72 MHz */


#define MOTOR_MIN_SPEED     50              
#define MOTOR_MAX_SPEED     1000            
#define MOTOR_RAMP_STEP     10              


#define CELL_SIZE_MM        180.0f          /* Maze cell size in mm */
#define TURN_ANGLE_DEG      90.0f           /* Turn angle in degrees */
      
#endif /* STM32_CONFIG_H */
