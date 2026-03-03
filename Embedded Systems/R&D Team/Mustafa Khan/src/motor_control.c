#include "motor_control.h"
#include <math.h>

static RobotMotors robot_motors = {0};
static float left_pwm_scale = 1.0f;
static float right_pwm_scale = 1.0f;
static float motor_speed_const_rpm_per_v = 1210.0f;
static float motor_torque_const_mnm_per_a = 7.92f;
static float velocity_to_duty_scale = 0.00586389f; /* (60*1000)/(pi*wheel_diam*gear*max_rpm) */
static void Motor_GPIO_Init(void);
static void Motor_Timer_Init(void);
static void Motor_ADC_Init(void);
static float Motor_MappingEncoderToDistance(int32_t encoder_counts);
static uint16_t Motor_SpeedToRPM(uint16_t speed_0_1000);

void Motor_Init(void) {
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | 
                           RCC_APB2Periph_GPIOC | RCC_APB2Periph_TIM1, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 | RCC_APB1Periph_TIM3, ENABLE);
    
    Motor_GPIO_Init();
    Motor_ConfigurePWM();
    Motor_ConfigureEncoders();
    Motor_ConfigureGate();
    
    robot_motors.left.enabled = false;
    robot_motors.right.enabled = false;
    robot_motors.left.direction = MOTOR_STOPPED;
    robot_motors.right.direction = MOTOR_STOPPED;
    robot_motors.battery_voltage = 12.0f;
    Motor_SetSpeedConstants(motor_speed_const_rpm_per_v, motor_torque_const_mnm_per_a);
}

/**
 * Configure GPIO pins for motor control
 */
static void Motor_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct;
    
    // PWM Output Pins (PA8, PA9, PA10, PA11) - TIM1 Alternate Function 
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;  /* Alternate function push-pull */
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    // Motor Enable Pins (PB3, PB4, PB5, PB6) - GPIO Output
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;  /* Output push-pull */
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
    

    GPIO_ResetBits(GPIOB, GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6);
    
    // Encoder Input Pins (PA0, PA1, PA2, PA3) - TIM Input Capture
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/**
 * Configure PWM on TIM1 for motor control
 * TIM1_CH1 (PA8):  Motor L_A
 * TIM1_CH2 (PA9):  Motor L_B
 * TIM1_CH3 (PA10): Motor R_B
 * TIM1_CH4 (PA11): Motor R_A
 */
void Motor_ConfigurePWM(void) {
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_OCInitTypeDef TIM_OCInitStruct;
    
    /* Calculate prescaler and period for 20kHz PWM at 72MHz APB2 */
    /* PWM_Freq = APB2_Freq / (Prescaler * (Period + 1)) */
    /* 20kHz = 72MHz / (Prescaler * (Period + 1)) */
    /* Using Prescaler=1, Period=3600: 20kHz = 72MHz / (1 * 3600) = 20kHz */
    
    TIM_TimeBaseInitStruct.TIM_Prescaler = 1 - 1;  /* Prescaler divides by 1 */
    TIM_TimeBaseInitStruct.TIM_Period = 3600 - 1;  /* Period = 3600, gives resolution of 1000 steps */
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStruct);
    
    /* Configure Output Compare for all 4 channels */
    TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;           /* PWM mode 1 */
    TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStruct.TIM_OutputNState = TIM_OutputNState_Disable;
    TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStruct.TIM_OCNPolarity = TIM_OCNPolarity_High;
    TIM_OCInitStruct.TIM_OCIdleState = TIM_OCIdleState_Reset;
    TIM_OCInitStruct.TIM_OCNIdleState = TIM_OCNIdleState_Reset;
    TIM_OCInitStruct.TIM_Pulse = 0;  /* Initial duty cycle = 0% */
    
    TIM_OC1Init(TIM1, &TIM_OCInitStruct);  /* CH1: PA8  */
    TIM_OC2Init(TIM1, &TIM_OCInitStruct);  /* CH2: PA9  */
    TIM_OC3Init(TIM1, &TIM_OCInitStruct);  /* CH3: PA10 */
    TIM_OC4Init(TIM1, &TIM_OCInitStruct);  /* CH4: PA11 */
    
    /* Enable preload registers */
    TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);
    TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);
    TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Enable);
    TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM1, ENABLE);
    
    /* Enable TIM1 counter */
    TIM_Cmd(TIM1, ENABLE);
    
    /* Enable TIM1 main output */
    TIM_CtrlPWMOutputs(TIM1, ENABLE);
}

void Motor_ConfigureEncoders(void) {
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_ICInitTypeDef TIM_ICInitStruct;
    
    /* === TIM2: Left Encoder === */
    TIM_TimeBaseInitStruct.TIM_Prescaler = 0;       /* No prescaling */
    TIM_TimeBaseInitStruct.TIM_Period = 0xFFFF;     /* 16-bit counter */
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStruct);
    
    /* TIM2 in quadrature mode */
    TIM_EncoderInterfaceConfig(TIM2, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, 
                                TIM_ICPolarity_Rising);
    TIM_Cmd(TIM2, ENABLE);
    
    /* === TIM3: Right Encoder === */
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStruct);
    TIM_EncoderInterfaceConfig(TIM3, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, 
                                TIM_ICPolarity_Rising);
    TIM_Cmd(TIM3, ENABLE);
    
    /* Reset encoder counters */
    TIM2->CNT = 0;
    TIM3->CNT = 0;
}

/**
 * Configure gate driver enable pins
 */
void Motor_ConfigureGate(void) {

    GPIO_ResetBits(GPIOB, GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6);
}

void Motor_SetDirection_Left(MotorDirection direction) {
    robot_motors.left.direction = direction;
    
    switch (direction) {
        case MOTOR_FORWARD:
            GPIO_SetBits(GPIOB, MOTOR_L_EN_A_PIN);      /* Enable IN_A_L */
            GPIO_ResetBits(GPIOB, MOTOR_L_EN_B_PIN);    /* Disable IN_B_L */
            break;
            
        case MOTOR_REVERSE:
            GPIO_ResetBits(GPIOB, MOTOR_L_EN_A_PIN);    /* Disable IN_A_L */
            GPIO_SetBits(GPIOB, MOTOR_L_EN_B_PIN);      /* Enable IN_B_L */
            break;
            
        case MOTOR_BRAKE:
            GPIO_SetBits(GPIOB, MOTOR_L_EN_A_PIN);      /* Enable both */
            GPIO_SetBits(GPIOB, MOTOR_L_EN_B_PIN);
            break;
            
        case MOTOR_STOPPED:
            GPIO_ResetBits(GPIOB, MOTOR_L_EN_A_PIN);    /* Disable both */
            GPIO_ResetBits(GPIOB, MOTOR_L_EN_B_PIN);
            break;
    }
}

/**
 * Set direction for right motor via gate driver enable pins
 */
void Motor_SetDirection_Right(MotorDirection direction) {
    robot_motors.right.direction = direction;
    
    switch (direction) {
        case MOTOR_FORWARD:
            GPIO_SetBits(GPIOB, MOTOR_R_EN_A_PIN);      /* Enable IN_A_R */
            GPIO_ResetBits(GPIOB, MOTOR_R_EN_B_PIN);    /* Disable IN_B_R */
            break;
            
        case MOTOR_REVERSE:
            GPIO_ResetBits(GPIOB, MOTOR_R_EN_A_PIN);    /* Disable IN_A_R */
            GPIO_SetBits(GPIOB, MOTOR_R_EN_B_PIN);      /* Enable IN_B_R */
            break;
            
        case MOTOR_BRAKE:
            GPIO_SetBits(GPIOB, MOTOR_R_EN_A_PIN);      /* Enable both */
            GPIO_SetBits(GPIOB, MOTOR_R_EN_B_PIN);
            break;
            
        case MOTOR_STOPPED:
            GPIO_ResetBits(GPIOB, MOTOR_R_EN_A_PIN);    /* Disable both */
            GPIO_ResetBits(GPIOB, MOTOR_R_EN_B_PIN);
            break;
    }
}

void Motor_SetPWM_Left(uint16_t duty) {
    uint16_t calibrated_duty;

    if (duty > 1000) duty = 1000;
    calibrated_duty = (uint16_t)((float)duty * left_pwm_scale);
    if (calibrated_duty > 1000) calibrated_duty = 1000;
    robot_motors.left.pwm_duty = calibrated_duty;
    
    /* Scale duty 0-1000 to PWM value 0-3600 */
    uint32_t pwm_value = (calibrated_duty * 3600) / 1000;
    TIM_SetCompare1(TIM1, pwm_value);  /* PA8 */
}


void Motor_SetPWM_Right(uint16_t duty) {
    uint16_t calibrated_duty;

    if (duty > 1000) duty = 1000;
    calibrated_duty = (uint16_t)((float)duty * right_pwm_scale);
    if (calibrated_duty > 1000) calibrated_duty = 1000;
    robot_motors.right.pwm_duty = calibrated_duty;
    
    /* Scale duty 0-1000 to PWM value 0-3600 */
    uint32_t pwm_value = (calibrated_duty * 3600) / 1000;
    TIM_SetCompare4(TIM1, pwm_value);  /* PA11 */
}

int32_t Motor_GetEncoderCount_Left(void) {
    return (int32_t)TIM2->CNT;
}

int32_t Motor_GetEncoderCount_Right(void) {
    return (int32_t)TIM3->CNT;
}

void Motor_ResetEncoder_Left(void) {
    TIM2->CNT = 0;
    robot_motors.left.pulse_count = 0;
    robot_motors.left.distance_mm = 0;
}

void Motor_ResetEncoder_Right(void) {
    TIM3->CNT = 0;
    robot_motors.right.pulse_count = 0;
    robot_motors.right.distance_mm = 0;
}

void Motor_Forward(uint16_t speed) {
    Motor_SetDirection_Left(MOTOR_FORWARD);
    Motor_SetDirection_Right(MOTOR_FORWARD);
    Motor_SetPWM_Left(speed);
    Motor_SetPWM_Right(speed);
    robot_motors.left.enabled = true;
    robot_motors.right.enabled = true;
}

void Motor_Backward(uint16_t speed) {
    Motor_SetDirection_Left(MOTOR_REVERSE);
    Motor_SetDirection_Right(MOTOR_REVERSE);
    Motor_SetPWM_Left(speed);
    Motor_SetPWM_Right(speed);
    robot_motors.left.enabled = true;
    robot_motors.right.enabled = true;
}

void Motor_Stop(void) {
    Motor_SetPWM_Left(0);
    Motor_SetPWM_Right(0);
    Motor_SetDirection_Left(MOTOR_STOPPED);
    Motor_SetDirection_Right(MOTOR_STOPPED);
    robot_motors.left.enabled = false;
    robot_motors.right.enabled = false;
}

void Motor_Brake(void) {
    Motor_SetPWM_Left(0);
    Motor_SetPWM_Right(0);
    Motor_SetDirection_Left(MOTOR_BRAKE);
    Motor_SetDirection_Right(MOTOR_BRAKE);
}

/**
 * Turn left (right wheel faster, left wheel slower)
 * speed: 0-1000
 */
void Motor_TurnLeft(uint16_t speed) {
    uint16_t speed_slow = (speed * 50) / 100;  /* Left motor at 50% speed */
    
    Motor_SetDirection_Left(MOTOR_FORWARD);
    Motor_SetDirection_Right(MOTOR_FORWARD);
    Motor_SetPWM_Left(speed_slow);
    Motor_SetPWM_Right(speed);
    robot_motors.left.enabled = true;
    robot_motors.right.enabled = true;
}

/**
 * Turn right (left wheel faster, right wheel slower)
 * speed: 0-1000
 */
void Motor_TurnRight(uint16_t speed) {
    uint16_t speed_slow = (speed * 50) / 100;  /* Right motor at 50% speed */
    
    Motor_SetDirection_Left(MOTOR_FORWARD);
    Motor_SetDirection_Right(MOTOR_FORWARD);
    Motor_SetPWM_Left(speed);
    Motor_SetPWM_Right(speed_slow);
    robot_motors.left.enabled = true;
    robot_motors.right.enabled = true;
}

void Motor_SpinLeft(uint16_t speed) {
    Motor_SetDirection_Left(MOTOR_REVERSE);
    Motor_SetDirection_Right(MOTOR_FORWARD);
    Motor_SetPWM_Left(speed);
    Motor_SetPWM_Right(speed);
    robot_motors.left.enabled = true;
    robot_motors.right.enabled = true;
}

void Motor_SpinRight(uint16_t speed) {
    Motor_SetDirection_Left(MOTOR_FORWARD);
    Motor_SetDirection_Right(MOTOR_REVERSE);
    Motor_SetPWM_Left(speed);
    Motor_SetPWM_Right(speed);
    robot_motors.left.enabled = true;
    robot_motors.right.enabled = true;
}

MotorState* Motor_GetState_Left(void) {
    robot_motors.left.pulse_count = Motor_GetEncoderCount_Left();
    robot_motors.left.distance_mm = Motor_MappingEncoderToDistance(robot_motors.left.pulse_count);
    return &robot_motors.left;
}

MotorState* Motor_GetState_Right(void) {
    robot_motors.right.pulse_count = Motor_GetEncoderCount_Right();
    robot_motors.right.distance_mm = Motor_MappingEncoderToDistance(robot_motors.right.pulse_count);
    return &robot_motors.right;
}

/**
 * Get all motor states
 */
RobotMotors* Motor_GetAllStates(void) {
    Motor_GetState_Left();
    Motor_GetState_Right();
    return &robot_motors;
}

float Motor_GetDistance_Left(void) {
    return Motor_MappingEncoderToDistance(Motor_GetEncoderCount_Left());
}

float Motor_GetDistance_Right(void) {
    return Motor_MappingEncoderToDistance(Motor_GetEncoderCount_Right());
}

float Motor_GetDistance_Combined(void) {
    float left_dist = Motor_GetDistance_Left();
    float right_dist = Motor_GetDistance_Right();
    return (left_dist + right_dist) / 2.0f;
}

static float Motor_MappingEncoderToDistance(int32_t encoder_counts) {
    float revolutions = (float)encoder_counts / (float)ENCODER_COUNTS_PER_REV;
    float distance = revolutions * WHEEL_CIRCUMFERENCE_MM / GEAR_RATIO;
    return distance;
}

static uint16_t Motor_SpeedToRPM(uint16_t speed_0_1000) {
    return (speed_0_1000 * 14100) / 1000;
}

void Motor_Calibrate(void) {
    const uint16_t calibration_pwm = 500;
    const uint32_t calibration_wait_cycles = 250000;
    int32_t left_counts, right_counts;
    float ratio;
    uint32_t i;

    left_pwm_scale = 1.0f;
    right_pwm_scale = 1.0f;

    Motor_ResetEncoder_Left();
    Motor_ResetEncoder_Right();
    Motor_Forward(calibration_pwm);

    for (i = 0; i < calibration_wait_cycles; i++) {
        __NOP();
    }

    Motor_Stop();

    left_counts = Motor_GetEncoderCount_Left();
    right_counts = Motor_GetEncoderCount_Right();
    if (left_counts < 0) left_counts = -left_counts;
    if (right_counts < 0) right_counts = -right_counts;

    if (left_counts == 0 || right_counts == 0) {
        return;
    }

    if (left_counts > right_counts) {
        ratio = (float)left_counts / (float)right_counts;
        if (ratio > 1.25f) ratio = 1.25f;
        right_pwm_scale = ratio;
    } else if (right_counts > left_counts) {
        ratio = (float)right_counts / (float)left_counts;
        if (ratio > 1.25f) ratio = 1.25f;
        left_pwm_scale = ratio;
    }
}


void Motor_SetSpeedConstants(float speed_const, float torque_const) {
    float max_rpm;

    if (speed_const > 0.0f) {
        motor_speed_const_rpm_per_v = speed_const;
    }
    if (torque_const > 0.0f) {
        motor_torque_const_mnm_per_a = torque_const;
    }

    max_rpm = motor_speed_const_rpm_per_v * robot_motors.battery_voltage;
    if (max_rpm > 0.0f) {
        velocity_to_duty_scale = (60.0f * 1000.0f) /
                                 (3.14159f * WHEEL_DIAMETER_MM * GEAR_RATIO * max_rpm);
    }
}


void Motor_SetVelocity_Left(float velocity_mms) {
    robot_motors.left.target_velocity_mms = velocity_mms;
    float abs_velocity = (velocity_mms >= 0.0f) ? velocity_mms : -velocity_mms;
    uint16_t duty = (uint16_t)(abs_velocity * velocity_to_duty_scale);
    if (duty > 1000) duty = 1000;
    
    Motor_SetPWM_Left(duty);
    Motor_SetDirection_Left(velocity_mms >= 0 ? MOTOR_FORWARD : MOTOR_REVERSE);
}


void Motor_SetVelocity_Right(float velocity_mms) {
    robot_motors.right.target_velocity_mms = velocity_mms;
    float abs_velocity = (velocity_mms >= 0.0f) ? velocity_mms : -velocity_mms;
    uint16_t duty = (uint16_t)(abs_velocity * velocity_to_duty_scale);
    if (duty > 1000) duty = 1000;
    
    Motor_SetPWM_Right(duty);
    Motor_SetDirection_Right(velocity_mms >= 0 ? MOTOR_FORWARD : MOTOR_REVERSE);
}


void Motor_SetVelocity_Both(float velocity_mms) {
    Motor_SetVelocity_Left(velocity_mms);
    Motor_SetVelocity_Right(velocity_mms);
}
