#include <mpu6050.h>
#include "stm32f1xx_hal_gpio.h"
#include <stdio.h>
#include <string.h>

extern I2C_HandleTypeDef hi2c2;
extern UART_HandleTypeDef huart2;

int mpu6050_init() {
    // left shift MPU Address for read/write bit
    HAL_StatusTypeDef isReady = HAL_I2C_IsDeviceReady(&hi2c2, (MPU_ADDRESS << 1) + 0, 1, 100);
    if (isReady == HAL_OK) { 
        // writing to registers responsible for configuring gyroscope/accelerometer scale & sleep mode
        HAL_StatusTypeDef gyro_config = HAL_I2C_Mem_Write(&hi2c2, MPU_ADDRESS << 1, 27, 1,REG_GYRO_0500, 1, 100);
        HAL_StatusTypeDef accel_config = HAL_I2C_Mem_Write(&hi2c2, MPU_ADDRESS << 1, 28, 1,REG_ACCEL_04, 1, 100);
        HAL_StatusTypeDef power_config = HAL_I2C_Mem_Write(&hi2c2, MPU_ADDRESS << 1, 107, 1,REG_TEMP_DIS, 1, 100);

        if (gyro_config != HAL_OK || accel_config != HAL_OK || 
            power_config != HAL_OK) { // if config failed flash onboard LED
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
            HAL_Delay(5000);
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
            return 0;
        } 
    }
    else {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
    }
    return 1;
}

void write_over_UART(uint16_t x, uint16_t y, uint16_t z) {
    //x values
    char x_message[] = "x axis:";
    HAL_UART_Transmit(&huart2, (uint8_t*)x_message, strlen(x_message), HAL_MAX_DELAY);
    //Transmit over UART
    char x_data_bus[16];//of size (x_accel)
    sprintf(x_data_bus, "%u\r\n", x);
    //strlen is the length of data_bus
    HAL_UART_Transmit(&huart2, (uint8_t*)x_data_bus, strlen(x_data_bus), HAL_MAX_DELAY);

    //y values
    char y_message[] = "y axis:";
    HAL_UART_Transmit(&huart2, (uint8_t*)y_message, strlen(y_message), HAL_MAX_DELAY);
    
    char y_data_bus[16];
    sprintf(y_data_bus, "%u\r\n", y);
    HAL_UART_Transmit(&huart2, (uint8_t*)y_data_bus, strlen(y_data_bus), HAL_MAX_DELAY);
    
    //z values
    char z_message[] = "z axis:";
    HAL_UART_Transmit(&huart2, (uint8_t*)z_message, strlen(z_message), HAL_MAX_DELAY);

    char z_data_bus[16];
    sprintf(z_data_bus, "%u\r\n", z);
    HAL_UART_Transmit(&huart2, (uint8_t*)z_data_bus, strlen(z_data_bus), HAL_MAX_DELAY);
}

void mpu6050_mem_read() {
    // A2 - Tx, A3 - Rx, B10 - SCL, B11 - SDA
    //gyro
    int16_t x_accel, y_accel, z_accel;
    uint8_t accel_output[6]; //each axis provides 2 bytes of data,
    HAL_I2C_Mem_Read(&hi2c2, (MPU_ADDRESS << 1) + 1, ACCEL_REG, 1, accel_output, 6, 100);
    x_accel = ((int16_t)accel_output[0] << 8) + accel_output[1];
    y_accel = ((int16_t)accel_output[2] << 8) + accel_output[3];
    z_accel = ((int16_t)accel_output[4] << 8) + accel_output[5];
    HAL_Delay(1000);

    write_over_UART(x_accel, y_accel, z_accel);

    //accel
    int16_t x_gyro, y_gyro, z_gyro;
    uint8_t gyro_output[6]; //each axis provides 2 bytes of data,
    HAL_I2C_Mem_Read(&hi2c2, (MPU_ADDRESS << 1) + 1, GYRO_REG, 1, gyro_output, 6, 100);
    x_gyro = ((int16_t)gyro_output[0] << 8) + gyro_output[1];
    y_gyro = ((int16_t)gyro_output[2] << 8) + gyro_output[3];
    z_gyro = ((int16_t)gyro_output[4] << 8) + gyro_output[5];
    HAL_Delay(1000);

    write_over_UART(x_gyro, y_gyro, z_gyro);
}
