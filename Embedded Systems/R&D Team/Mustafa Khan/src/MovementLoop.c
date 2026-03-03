#include "movement_controller.h"
#include "motor_control.h"
#include "stm32_config.h"

typedef enum {
    STATE_READY = 0,
    STATE_EXPLORING = 1,
    STATE_RUNNING = 2,
    STATE_COMPLETE = 3,
    STATE_ERROR = 4
} RobotState;

typedef struct {
    RobotState state;
    uint32_t loop_count;
    uint32_t loop_time_ms;
    bool emergency_stop;
} RobotControlState;

static RobotControlState robot_state = {STATE_READY, 0, 0, false};

extern int floodFill[16][16];
extern int** findWalls(void);
extern void findPath(int startX, int startY, int goalX, int goalY);

void MovementLoop_Init(void) {
    
    Movement_Init();
    Movement_ResetPose();
    
    robot_state.state = STATE_READY;
    robot_state.loop_count = 0;
    robot_state.emergency_stop = false;
}

void MovementLoop_Update(void) {
    if (robot_state.emergency_stop) {
        Motor_Stop();
        return;
    }
    
    robot_state.loop_count++;
    
    
    Movement_Update();
    
    switch (robot_state.state) {
        case STATE_READY:
            
            Motor_Stop();
            break;
            
        case STATE_EXPLORING:
            /* Robot is exploring the maze and building a map */
            /* This is handled by the pathfinding algorithm */
            if (Movement_IsMovementComplete()) {
                /* Current movement segment complete, get next instruction */
                /* TODO: Call pathfinding for next move */
            }
            break;
            
        case STATE_RUNNING:
            
            if (Movement_IsMovementComplete()) {
                /* Current movement segment complete */
                /* TODO: Get next waypoint from planned path */
            }
            break;
            
        case STATE_COMPLETE:
            Motor_Stop();
            break;
            
        case STATE_ERROR:
            Motor_Brake();
            break;
            
        default:
            Motor_Stop();
            break;
    }
}
void MovementLoop_StartExploration(void) {
    robot_state.state = STATE_EXPLORING;
    Movement_ResetPose();
    /* Flood-fill algorithm will guide navigation */
}

void MovementLoop_StartPathExecution(void) {
    robot_state.state = STATE_RUNNING;
}

void MovementLoop_EmergencyStop(void) {
    robot_state.emergency_stop = true;
    Motor_Brake();
}

void MovementLoop_Resume(void) {
    robot_state.emergency_stop = false;
}

void MovementLoop_MoveForward(uint16_t speed) {
    Movement_MoveForward_Cells(1, speed);
    
    /* Wait for movement to complete */
    while (Movement_IsMoving()) {
        MovementLoop_Update();
        /* In real implementation, add timeout and error checking */
        /* Use HAL_Delay or similar */
    }
}
/
void MovementLoop_MoveForward_Cells(uint8_t num_cells, uint16_t speed) {
    Movement_MoveForward_Cells(num_cells, speed);
    
    /* Wait for movement to complete */
    while (Movement_IsMoving()) {
        MovementLoop_Update();
    }
}

void MovementLoop_TurnLeft(uint16_t speed) {
    Movement_TurnLeft_90(speed);
    
    while (Movement_IsMoving()) {
        MovementLoop_Update();
    }
}

void MovementLoop_TurnRight(uint16_t speed) {
    Movement_TurnRight_90(speed);
    
    while (Movement_IsMoving()) {
        MovementLoop_Update();
    }
}

void MovementLoop_TurnAround(uint16_t speed) {
    Movement_TurnAround_180(speed);
    
    while (Movement_IsMoving()) {
        MovementLoop_Update();
    }
}

RobotPose* MovementLoop_GetPose(void) {
    return Movement_GetPose();
}

void MovementLoop_CorrectPose(float x_mm, float y_mm, Orientation facing) {
    Movement_SetPose(x_mm, y_mm, facing);
}

void MovementLoop_Calibrate(void) {
    Motor_Calibrate();
}

float MovementLoop_GetSpeedError(void) {
    float left = Motor_GetDistance_Left();
    float right = Motor_GetDistance_Right();
    return fabsf(left - right);
}

uint32_t MovementLoop_GetLoopCount(void) {
    return robot_state.loop_count;
}

void MovementLoop_PrintState(void) {
    RobotPose* pose = Movement_GetPose();
    MotorState* left = Motor_GetState_Left();
    MotorState* right = Motor_GetState_Right();
    
}


void MovementLoop_ExecuteMove(uint8_t direction, uint8_t moveType, uint16_t speed) {
    switch (moveType) {
        case 0:  /* Forward */
            MovementLoop_MoveForward(speed);
            break;
        case 1:  /* Turn left */
            MovementLoop_TurnLeft(speed);
            break;
        case 2:  /* Turn right */
            MovementLoop_TurnRight(speed);
            break;
        case 3:  /* U-turn */
            MovementLoop_TurnAround(speed);
            break;
    }
}

/**
 * Execute path from queue
 * Call this repeatedly to execute planned moves
 */
void MovementLoop_ExecutePath(void) {
    /* TODO: Get next move from path queue */
    
}
