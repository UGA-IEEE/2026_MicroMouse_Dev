
#include "stm32f1xx.h"
#include "stm32_config.h"
#include "motor_control.h"
#include "movement_controller.h"
#include <stdint.h>
#include <stdbool.h>
#include <math.h>


/* tick frequency - 1ms per tick */
#define SYSTICK_FREQ_HZ     1000
volatile uint32_t sys_tick_ms = 0;

/* Main loop frequency */
#define MAIN_LOOP_FREQ_HZ   100
#define MAIN_LOOP_PERIOD_MS 10

/* Maze parameters */
#define MAZE_SIZE 16
#define CELL_SIZE_MM 180  /* Standard IEEE maze cell is 180mm */


typedef enum {
    STATE_INIT,
    STATE_READY,
    STATE_EXPLORING,
    STATE_RETURNING,
    STATE_COMPLETE
} RobotState;

static RobotState robot_state = STATE_INIT;

/* Robot position in maze coordinates (0-15 for each axis) */
typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t facing;  /* 0=East, 1=South, 2=West, 3=North */
} MazePosition;

static MazePosition maze_pos = {0, 0, 0};  /* Start at (0,0) facing East */

void SysTick_Handler(void) {
    sys_tick_ms++;
}

void SystemClockInit(void) {
    /* Enable HSE oscillator */
    RCC->CR |= RCC_CR_HSEON;
    while (!(RCC->CR & RCC_CR_HSERDY));
    
    /* Configure PLL (8MHz HSE * 9 = 72MHz) */
    RCC->CFGR |= RCC_CFGR_PLLSRC;           /* HSE as PLL source */
    RCC->CFGR |= RCC_CFGR_PLLMULL9;         /* Multiply by 9 */
    
    /* Enable PLL */
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY));
    
    /* Select PLL as system clock */
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
    
    /* Configure flash latency for 72MHz */
    FLASH->ACR = FLASH_ACR_LATENCY_2;
    
    /* Update bus prescalers */
    RCC->CFGR |= RCC_CFGR_HPRE_DIV1;   /* AHB = 72MHz */
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;  /* APB1 = 36MHz */
    RCC->CFGR |= RCC_CFGR_PPRE2_DIV1;  /* APB2 = 72MHz */
}


void SysTickInit(void) {
    /* SysTick for 1ms tick at 72MHz */
    SysTick_Config(SYSTEM_CLOCK_FREQ / SYSTICK_FREQ_HZ);
    NVIC_SetPriority(SysTick_IRQn, 3);  /* Low priority */
}

void System_Init(void) {
    
    SystemClockInit();
    SysTickInit();
    
    
    Motor_Init();
    Movement_Init();
    
    robot_state = STATE_READY;
}


uint32_t get_time_ms(void) {
    return sys_tick_ms;
}


void delay_ms(uint32_t ms) {
    uint32_t start = sys_tick_ms;
    while (sys_tick_ms - start < ms);
}

/**
 * Convert maze cell distance to millimeters
 */
float cells_to_mm(uint8_t cells) {
    return (float)cells * CELL_SIZE_MM;
}


bool move_forward_cells(uint8_t num_cells) {
    if (!Movement_IsMoving()) {
        
        Movement_MoveForward_Cells(num_cells, 700);  /* Speed: 700/1000 = 70% */
    }
    
    if (Movement_IsMoving()) {
        Movement_Update();
        return false;  
    } else {
        return true;  
    }
}


bool turn_left_90(void) {
    if (!Movement_IsMoving()) {
        Movement_TurnLeft_90(600);  
    }
    
    if (Movement_IsMoving()) {
        Movement_Update();
        return false;
    } else {
        return true;
    }
}


bool turn_right_90(void) {
    if (!Movement_IsMoving()) {
        Movement_TurnRight_90(600);  
    }
    
    if (Movement_IsMoving()) {
        Movement_Update();
        return false;
    } else {
        return true;
    }
}


bool turn_around_180(void) {
    static bool first_turn = true;
    
    if (first_turn) {
        if (!turn_left_90()) {
            return false;  
        }
        first_turn = false;
        return false;  
    } else {
        if (!turn_left_90()) {
            return false;  /* Second 90 deg turn still in progress */
        }
        first_turn = true;
        return true;   
    }
}


void update_maze_position(uint8_t move_direction) {
    
    switch (move_direction) {
        case 0:  /* Forward in current facing direction */
            if (maze_pos.facing == 0) maze_pos.x++;      /* East */
            else if (maze_pos.facing == 1) maze_pos.y++; /* South */
            else if (maze_pos.facing == 2) maze_pos.x--; /* West */
            else if (maze_pos.facing == 3) maze_pos.y--; /* North */
            break;
            
        case 2:  /* Turn left (facing changes, position stays same) */
            maze_pos.facing = (maze_pos.facing + 3) % 4;
            break;
            
        case 3:  /* Turn right (facing changes, position stays same) */
            maze_pos.facing = (maze_pos.facing + 1) % 4;
            break;
    }
}


void display_robot_state(void) {
    RobotPose* pose = Movement_GetPose();
    
}


void test_motor_basic(void) {
    uint32_t test_start = sys_tick_ms;
    
    /* Move forward 1 cell */
    Movement_MoveForward_Cells(1, 700);
    while (Movement_IsMoving() && (sys_tick_ms - test_start < 10000)) {
        Movement_Update();
    }
    Motor_Stop();
    
    
    delay_ms(500);
    
    /* Turn right 90 degrees */
    Movement_TurnRight_90(600);
    test_start = sys_tick_ms;
    while (Movement_IsMoving() && (sys_tick_ms - test_start < 5000)) {
        Movement_Update();
    }
    Motor_Stop();
}


int main(void) {
    uint32_t last_loop_time = 0;
    
    /* Initialize system */
    System_Init();

    
    test_motor_basic();     */  /* Test single movement */
    
    /* Main control loop */
    while (1) {
        
        if (sys_tick_ms - last_loop_time >= MAIN_LOOP_PERIOD_MS) {
            last_loop_time = sys_tick_ms;
            
            
            Movement_Update();
            
            
            switch (robot_state) {
                case STATE_READY:
                    /* Waiting for command - could transition to EXPLORING */
                    robot_state = STATE_EXPLORING;
                    break;
                    
                case STATE_EXPLORING:
                    /* Main maze solving loop
                     * 
                     *                  
                    /* Placeholder: Just idle in exploring state */
                    break;
                    
                case STATE_RETURNING:
                    
                    robot_state = STATE_COMPLETE;
                    break;
                    
                case STATE_COMPLETE:
                    
                    Motor_Stop();
                    delay_ms(1);
                    break;
                    
                default:
                    break;
            }
        }
    }
    
    return 0;
}

