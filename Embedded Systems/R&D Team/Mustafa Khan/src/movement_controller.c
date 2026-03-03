#include "movement_controller.h"
#include <math.h>
#include <string.h>

static RobotPose robot_pose = {0};
static MovementCommand current_move = {0};

/* === Private Function Prototypes === */
static float Movement_NormalizeAngle(float angle_deg);
static uint32_t Movement_EstimateTime_Movement(float distance_mm, uint16_t speed);
static uint32_t Movement_EstimateTime_Rotation(float angle_deg, uint16_t speed);

void Movement_Init(void) {
    Motor_Init();
    
    /* Initialize robot pose */
    robot_pose.x_mm = 0.0f;
    robot_pose.y_mm = 0.0f;
    robot_pose.theta_deg = 0.0f;  /* Facing EAST initially */
    robot_pose.facing = FACING_EAST;
    robot_pose.linear_velocity_mms = 0.0f;
    robot_pose.angular_velocity_dps = 0.0f;
    
    current_move.state = MOVE_IDLE;
    current_move.target_distance_mm = 0.0f;
    current_move.target_angle_deg = 0.0f;
    current_move.max_speed = 500;
    current_move.acceleration = 50;
    

}

RobotPose* Movement_GetPose(void) {
    Movement_UpdatePose();
    return &robot_pose;
}

void Movement_UpdatePose(void) {
    /* Read encoder distances */
    float dist_left = Motor_GetDistance_Left();
    float dist_right = Motor_GetDistance_Right();
    float avg_distance = (dist_left + dist_right) / 2.0f;
    
    /* Calculate change in position */
    float delta_dist = avg_distance - robot_pose.x_mm;
    
    /* Update position based on orientation */
    switch (robot_pose.facing) {
        case FACING_EAST:
            robot_pose.x_mm += delta_dist;
            break;
        case FACING_SOUTH:
            robot_pose.y_mm += delta_dist;
            break;
        case FACING_WEST:
            robot_pose.x_mm -= delta_dist;
            break;
        case FACING_NORTH:
            robot_pose.y_mm -= delta_dist;
            break;
    }
    
    /* Calculate angular change from wheel difference */
    float encoder_diff = dist_right - dist_left;
    float angle_change = (encoder_diff / WHEEL_BASE_MM) * 57.2958f;  /* radians to degrees */
    robot_pose.theta_deg = Movement_NormalizeAngle(robot_pose.theta_deg + angle_change);
    

    MotorState* left_state = Motor_GetState_Left();
    MotorState* right_state = Motor_GetState_Right();
    robot_pose.linear_velocity_mms = (left_state->velocity_mms + right_state->velocity_mms) / 2.0f;
}


void Movement_SetPose(float x_mm, float y_mm, Orientation facing) {
    robot_pose.x_mm = x_mm;
    robot_pose.y_mm = y_mm;
    robot_pose.facing = facing;
    robot_pose.theta_deg = facing * 90.0f;  /* Convert enum to degrees */
    
    /* Reset motor encoders to match new position */
    Motor_ResetEncoder_Left();
    Motor_ResetEncoder_Right();
}

/**
 * Reset pose to origin facing EAST
 */
void Movement_ResetPose(void) {
    Movement_SetPose(0.0f, 0.0f, FACING_EAST);
}

void Movement_MoveForward_MM(float distance_mm, uint16_t speed) {
    /* Reset encoders */
    Motor_ResetEncoder_Left();
    Motor_ResetEncoder_Right();
    
    current_move.target_distance_mm = distance_mm;
    current_move.target_angle_deg = 0.0f;
    current_move.max_speed = speed;
    current_move.state = MOVE_FORWARD;
    current_move.start_time_ms = 0;  /* TODO: Get system time */
    current_move.estimated_time_ms = Movement_EstimateTime_Movement(distance_mm, speed);

    Motor_Forward(speed);
}
void Movement_MoveBackward_MM(float distance_mm, uint16_t speed) {
    Motor_ResetEncoder_Left();
    Motor_ResetEncoder_Right();
    
    current_move.target_distance_mm = distance_mm;
    current_move.target_angle_deg = 0.0f;
    current_move.max_speed = speed;
    current_move.state = MOVE_BACKWARD;
    current_move.start_time_ms = 0;
    current_move.estimated_time_ms = Movement_EstimateTime_Movement(distance_mm, speed);
    
    Motor_Backward(speed);
}

void Movement_RotateLeft_Degrees(float angle_deg, uint16_t speed) {
    Motor_ResetEncoder_Left();
    Motor_ResetEncoder_Right();
    
    current_move.target_distance_mm = 0.0f;
    current_move.target_angle_deg = angle_deg;
    current_move.max_speed = speed;
    current_move.state = MOVE_TURNING;
    current_move.start_time_ms = 0;
    current_move.estimated_time_ms = Movement_EstimateTime_Rotation(angle_deg, speed);
    
    Motor_SpinLeft(speed);
}

void Movement_RotateRight_Degrees(float angle_deg, uint16_t speed) {
    Motor_ResetEncoder_Left();
    Motor_ResetEncoder_Right();
    
    current_move.target_distance_mm = 0.0f;
    current_move.target_angle_deg = angle_deg;
    current_move.max_speed = speed;
    current_move.state = MOVE_TURNING;
    current_move.start_time_ms = 0;
    current_move.estimated_time_ms = Movement_EstimateTime_Rotation(angle_deg, speed);
    
    Motor_SpinRight(speed);
}

void Movement_Stop(void) {
    Motor_Stop();
    current_move.state = MOVE_IDLE;
}

void Movement_MoveForward_Cells(uint8_t num_cells, uint16_t speed) {
    float distance_mm = (float)num_cells * CELL_SIZE_MM;
    Movement_MoveForward_MM(distance_mm, speed);
}


void Movement_TurnLeft_90(uint16_t speed) {
    Movement_RotateLeft_Degrees(90.0f, speed);
    
    /* Update facing direction */
    robot_pose.facing = (Orientation)((robot_pose.facing + 3) % 4);  /* -1 mod 4 */
    robot_pose.theta_deg = Movement_NormalizeAngle(robot_pose.theta_deg - 90.0f);
}

void Movement_TurnRight_90(uint16_t speed) {
    Movement_RotateRight_Degrees(90.0f, speed);
    
    /* Update facing direction */
    robot_pose.facing = (Orientation)((robot_pose.facing + 1) % 4);
    robot_pose.theta_deg = Movement_NormalizeAngle(robot_pose.theta_deg + 90.0f);
}

void Movement_TurnAround_180(uint16_t speed) {
    Movement_RotateLeft_Degrees(180.0f, speed);
    
    /* Update facing direction */
    robot_pose.facing = (Orientation)((robot_pose.facing + 2) % 4);
    robot_pose.theta_deg = Movement_NormalizeAngle(robot_pose.theta_deg + 180.0f);
}

MovementState Movement_GetState(void) {
    return current_move.state;
}

bool Movement_IsMoving(void) {
    return (current_move.state != MOVE_IDLE && current_move.state != MOVE_COMPLETE);
}

bool Movement_IsMovementComplete(void) {
    Movement_UpdatePose();
    
    switch (current_move.state) {
        case MOVE_FORWARD:
        case MOVE_BACKWARD:
            return Motor_GetDistance_Combined() >= current_move.target_distance_mm - LINEAR_TOLERANCE_MM;
            
        case MOVE_TURNING:
            return fabsf(robot_pose.theta_deg) >= current_move.target_angle_deg - ANGULAR_TOLERANCE_DEG;
            
        default:
            return false;
    }
}


float Movement_GetProgress(void) {
    if (current_move.state == MOVE_IDLE) return 0.0f;
    
    float progress = 0.0f;
    
    if (current_move.state == MOVE_FORWARD || current_move.state == MOVE_BACKWARD) {
        float distance = Motor_GetDistance_Combined();
        progress = distance / current_move.target_distance_mm;
    } else if (current_move.state == MOVE_TURNING) {
        float angle = fabsf(robot_pose.theta_deg);
        progress = angle / current_move.target_angle_deg;
    }
    
    if (progress > 1.0f) progress = 1.0f;
    return progress;
}

static float Movement_NormalizeAngle(float angle_deg) {
    while (angle_deg < 0.0f) angle_deg += 360.0f;
    while (angle_deg >= 360.0f) angle_deg -= 360.0f;
    return angle_deg;
}

static uint32_t Movement_EstimateTime_Movement(float distance_mm, uint16_t speed) {
    /* Max speed ~235 mm/s (14100 RPM with 32mm wheel) */
    float velocity_mms = (speed / 1000.0f) * 235.0f;
    if (velocity_mms < 1.0f) velocity_mms = 1.0f;
    
    uint32_t time_ms = (uint32_t)(distance_mm / velocity_mms * 1000.0f);
    return time_ms;
}

static uint32_t Movement_EstimateTime_Rotation(float angle_deg, uint16_t speed) {
    /* Robot spins at approximately (speed/1000) * max_angular_velocity */
    /* Max angular velocity: ~470 deg/s (for 90mm wheelbase, 235 mm/s max) */
    float angular_velocity_dps = (speed / 1000.0f) * 470.0f;
    if (angular_velocity_dps < 1.0f) angular_velocity_dps = 1.0f;
    
    uint32_t time_ms = (uint32_t)(angle_deg / angular_velocity_dps * 1000.0f);
    return time_ms;
}

void Movement_Update(void) {
    if (Movement_IsMoving()) {
        if (Movement_IsMovementComplete()) {
            Motor_Stop();
            current_move.state = MOVE_COMPLETE;
        }
    }
}
