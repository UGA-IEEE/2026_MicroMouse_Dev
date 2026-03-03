#ifndef MOVEMENT_CONTROLLER_H
#define MOVEMENT_CONTROLLER_H

#include "motor_control.h"
#include "stm32_config.h"
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    MOVE_IDLE = 0,
    MOVE_FORWARD = 1,
    MOVE_BACKWARD = 2,
    MOVE_TURNING = 3,
    MOVE_COMPLETE = 4,
    MOVE_ERROR = 5
} MovementState;

typedef enum {
    FACING_EAST = 0,
    FACING_SOUTH = 1,
    FACING_WEST = 2,
    FACING_NORTH = 3
} Orientation;

typedef struct {
    float x_mm;                  /* Robot position X (mm) */
    float y_mm;                  /* Robot position Y (mm) */
    float theta_deg;             /* Robot orientation (degrees, 0-360) */
    Orientation facing;          /* Cardinal direction facing */
    float linear_velocity_mms;   /* Current linear velocity (mm/s) */
    float angular_velocity_dps;  /* Current angular velocity (deg/s) */
} RobotPose;

typedef struct {
    float target_distance_mm;    /* Target distance for movement */
    float target_angle_deg;      /* Target angle for rotation */
    float max_speed;             /* Max speed 0-1000 */
    float acceleration;          /* Acceleration ramp 0-1000 per 100ms */
    uint32_t start_time_ms;      /* Movement start time */
    uint32_t estimated_time_ms;  /* Estimated completion time */
    MovementState state;         /* Current movement state */
} MovementCommand;

void Movement_Init(void);


RobotPose* Movement_GetPose(void);
void Movement_UpdatePose(void);
void Movement_SetPose(float x_mm, float y_mm, Orientation facing);
void Movement_ResetPose(void);


void Movement_MoveForward_MM(float distance_mm, uint16_t speed);
void Movement_MoveBackward_MM(float distance_mm, uint16_t speed);
void Movement_RotateLeft_Degrees(float angle_deg, uint16_t speed);
void Movement_RotateRight_Degrees(float angle_deg, uint16_t speed);
void Movement_Stop(void);


void Movement_MoveForward_Cells(uint8_t num_cells, uint16_t speed);
void Movement_TurnLeft_90(uint16_t speed);
void Movement_TurnRight_90(uint16_t speed);
void Movement_TurnAround_180(uint16_t speed);


MovementState Movement_GetState(void);
bool Movement_IsMoving(void);
bool Movement_IsMovementComplete(void);
float Movement_GetProgress(void);



#endif 
