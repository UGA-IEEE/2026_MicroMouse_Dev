#ifndef MPU6050_H
#define MPU6050_H

#define MPU_ADDRESS 0b1101000

#define REG_GYRO_CONFIG 27
#define REG_ACCEL_CONFIG 28
#define REG_TEMP_CONFIG 107

#define ACCEL_REG 59
#define GYRO_REG 67

#define REG_GYRO_0250 0b00000000 
#define REG_GYRO_0500 0b00001000 
#define REG_GYRO_1000 0b00010000 
#define REG_GYRO_2000 0b00011000 

#define REG_ACCEL_02 0b00000000 
#define REG_ACCEL_04 0b00001000 
#define REG_ACCEL_08 0b00010000 
#define REG_ACCEL_16 0b00011000 

#define REG_TEMP_DIS 0b00001000



bool mpu6050_init();

void mpu6050_mem_read();

void write_over_UART(uint16_t x, uint16_t y, uint16_t z);




#endif