#include "bno085.h"
#include "hc06.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_gpio.h"
#include "stm32f1xx_hal_i2c.h"

// SHTP Header and Information
typedef struct {
    uint8_t lsbLength;
    uint8_t msbLength;
    uint8_t channel;
    uint8_t sequence;
} shtpHeader;

shtpHeader header;
uint8_t RxShtpData[512] = {0x00};
uint8_t TxShtpData[512] = {0x00};
uint8_t sequence[6] = {0};

extern I2C_HandleTypeDef hi2c1;

/*
* Checks I2C connection, restarts the BNO085, requests sensor ID, and enables the gyroscope.
*/
bool bno085_init() {
    HAL_Delay(50);

    if (HAL_I2C_IsDeviceReady(&hi2c1, BNO085_ADDRESS << 1, 10, 1000)
    != HAL_OK) return false;

    HAL_Delay(50);
    softReset();

    if (!requestID()) return false; 
        
    enableGyro();
    return true;
}

/*
* Sends and receives a Product ID Request/Response. Page #40 on SH-2 Manual
*/
bool requestID() {
    TxShtpData[0] = PRODUCTID_REQUEST;
    TxShtpData[1] = 0;

    if(!sendPacket(CHANNEL_CONTROL, 2, TxShtpData)) return false;
    if (!receivePacket()) return false;

    //if (RxShtpData[0] != PRODUCTID_RESPONSE) return false;
    return true;
}

/*
* Flushes sensor packets from the gyroscope. Page #66
*/
void flush() {
    TxShtpData[0] = FORCE_FLUSH_SENSOR;
    TxShtpData[1] = REPORTID_GYRO;

    sendPacket(CHANNEL_CONTROL, 2, TxShtpData);
}

/*
* Soft resets the Sensor to begin sending packets
*/
void softReset() {
    TxShtpData[0] = 1;
    sendPacket(CHANNEL_EXECUTABLE, 1, TxShtpData);
    receivePacket();

    HAL_Delay(100);
    flush();
}

/*
* Sends a header plus some data to the sensor as a packet
* Look at again
*/
bool sendPacket(uint8_t channelNumber, uint8_t dataLength, uint8_t *data) {
    uint8_t packet[MAX_PACKET_SIZE];

    packet[0] = (dataLength + 4) & 0xFF;
    packet[1] = ((dataLength + 4) >> 8) & 0xFF;
    packet[2] = channelNumber;

    packet[3] = sequence[channelNumber];
    sequence[channelNumber] += 1;

    memcpy(&packet[4], data, dataLength);

    if (HAL_I2C_Master_Transmit(&hi2c1, BNO085_ADDRESS << 1, packet, dataLength + 4, 100)
    != HAL_OK) {
       // clearBuffer(data);
        return false;
    }
  //  clearBuffer(data);
    return true;
}

/*
* Receives a header plus some data from the sensor.
* Look at again
*/
bool receivePacket() {
   // clearBuffer(RxShtpData);
    uint8_t headerPacket[4];
    uint16_t length;
    uint8_t msb;
    
    if (HAL_I2C_Master_Receive(&hi2c1, (BNO085_ADDRESS << 1) + 1, headerPacket, 4, 100)
    != HAL_OK) return false;
    //mask continuation bit
    length = ((headerPacket[1] << 8) | headerPacket[0]) & 0x7FFF;
    //assign continuation bit
    msb = (headerPacket[1] >> 7) & 1;

    //If the packet only contains the header return
    if (length <= 4) return false;
    length -= 4;

    header.lsbLength = headerPacket[0];
    header.msbLength = headerPacket[1] & 0x7F;
    header.channel = headerPacket[2];
    header.sequence = headerPacket[3];
    HAL_Delay(50);

    if (HAL_I2C_Master_Receive(&hi2c1, BNO085_ADDRESS << 1, RxShtpData, length, 100)
    != HAL_OK) return false;
    
    //Check for Continuation bit and do something later with that
   // if (msb == 1) {
     //   uint16_t s = sizeof(RxShtpData);
       // clearBuffer(RxShtpData);
        //HAL_I2C_Master_Receive(&hi2c1, BNO085_ADDRESS << 1, RxShtpData , length - s, 100);
    //}

    return true;
}

void clearBuffer(uint8_t *shtpData) {
    memset(shtpData, 0, 512);
}

/*
* Look at later
*/
bool enableGyro() {
    TxShtpData[0] = SET_FEATURE_COMMAND;
    TxShtpData[1] = REPORTID_GYRO;

    //ms to microseconds
    int reportInterval = 10 * 1000;

    TxShtpData[2] = 0x00;
    TxShtpData[3] = 0x00;
    TxShtpData[4] = 0x00;
    TxShtpData[5] = reportInterval & 0xFF;
    TxShtpData[6] = (reportInterval >> 8) & 0xFF;
    TxShtpData[7] = (reportInterval >> 16) & 0xFF;
    TxShtpData[8] = (reportInterval >> 24) & 0xFF;
    TxShtpData[9] = 0x00;
    TxShtpData[10] = 0x00;
    TxShtpData[11] = 0x00;
    TxShtpData[12] = 0x00;
    TxShtpData[13] = 0x00;
    TxShtpData[14] = 0x00;
    TxShtpData[15] = 0x00;
    TxShtpData[16] = 0x00;


   return sendPacket(CHANNEL_EXECUTABLE, 17, TxShtpData);
}
/*
* Fix, runs infinitely
*/
bool parseGyro(uint16_t *gyroAxis) {
    if (!receivePacket()) return false; //test first connection

    //Only look for input reports
    while (header.channel != CHANNEL_REPORTS) {
        receivePacket();
    }

    gyroAxis[0] = (RxShtpData[5] << 8) | RxShtpData[4]; //x axis
    gyroAxis[1] = (RxShtpData[7] << 8) | RxShtpData[6]; //y axis
    gyroAxis[2] = (RxShtpData[9] << 8) | RxShtpData[8]; //z axis

    return true;
}



