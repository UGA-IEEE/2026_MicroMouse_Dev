#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include "stm32f1xx.h"
#include "stm32_config.h"
#include <stdint.h>
#include <stdbool.h>

/* ============================================================================
 * Motor Control Driver for Faulhaber 1717T012SR Motors
 * Includes: PWM control, encoder reading, speed/distance tracking
 * ============================================================================ */

typedef enum {
    MOTOR_STOPPED = 0,
    MOTOR_FORWARD = 1,
    MOTOR_REVERSE = 2,
    MOTOR_BRAKE = 3
} MotorDirection;

typedef struct {
    int32_t pulse_count;        /* Current encoder pulse count */
    float distance_mm;           /* Distance traveled in mm */
    float velocity_mms;          /* Current velocity mm/s */
    float target_velocity_mms;   /* Target velocity mm/s */
    uint16_t pwm_duty;           /* Current PWM duty cycle (0-1000) */
    MotorDirection direction;    /* Current direction */
    bool enabled;                /* Motor enabled flag */
} MotorState;

typedef struct {
    MotorState left;
    MotorState right;
    float battery_voltage;       /* Battery voltage (V) */
} RobotMotors;

/* === Initialization === */
void Motor_Init(void);
void Motor_ConfigurePWM(void);
void Motor_ConfigureEncoders(void);
void Motor_ConfigureGate(void);

/* === Low-level Motor Control === */
void Motor_SetDirection_Left(MotorDirection direction);
void Motor_SetDirection_Right(MotorDirection direction);
void Motor_SetPWM_Left(uint16_t duty);      /* duty: 0-1000 */
void Motor_SetPWM_Right(uint16_t duty);     /* duty: 0-1000 */

/* === Encoder Reading === */
int32_t Motor_GetEncoderCount_Left(void);
int32_t Motor_GetEncoderCount_Right(void);
void Motor_ResetEncoder_Left(void);
void Motor_ResetEncoder_Right(void);

/* === Motor Movement Functions === */
void Motor_Forward(uint16_t speed);         /* speed: 0-1000 */
void Motor_Backward(uint16_t speed);        /* speed: 0-1000 */
void Motor_Stop(void);
void Motor_Brake(void);
void Motor_TurnLeft(uint16_t speed);        /* Left wheel slower */
void Motor_TurnRight(uint16_t speed);       /* Right wheel slower */
void Motor_SpinLeft(uint16_t speed);        /* Left wheel reverse */
void Motor_SpinRight(uint16_t speed);       /* Right wheel reverse */

/* === Distance-based Movement === */
void Motor_MoveForward_Distance(float distance_mm, uint16_t speed);
void Motor_TurnAngle(float angle_degrees, uint16_t speed);

/* === Velocity Control === */
void Motor_SetVelocity_Left(float velocity_mms);
void Motor_SetVelocity_Right(float velocity_mms);
void Motor_SetVelocity_Both(float velocity_mms);

/* === State Reading === */
MotorState* Motor_GetState_Left(void);
MotorState* Motor_GetState_Right(void);
RobotMotors* Motor_GetAllStates(void);
float Motor_GetDistance_Left(void);
float Motor_GetDistance_Right(void);
float Motor_GetDistance_Combined(void);

/* === Calibration & Tuning === */
void Motor_Calibrate(void);
void Motor_SetSpeedConstants(float speed_const, float torque_const);

#endif /* MOTOR_CONTROL_H */
