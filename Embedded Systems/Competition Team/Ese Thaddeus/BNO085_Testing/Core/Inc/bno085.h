#ifndef BNO085_H
#define BNO085_H

#include <stdbool.h>

#define BNO085_ADDRESS  0x4A

#define CHANNEL_COMMAND 0
#define CHANNEL_EXECUTABLE 1
#define CHANNEL_CONTROL 2
#define CHANNEL_REPORTS 3
#define CHANNEL_WAKE_REPORTS 4
#define CHANNEL_GYRO 5

#define COMMAND_REQUEST 0xF2
#define COMMAND_RESPONSE 0xF1
#define PRODUCTID_REQUEST 0xF9
#define PRODUCTID_RESPONSE 0xF8

#define REPORTID_ACCEL 0x01
#define REPORTID_GYRO 0x02
#define REPORTID_MAGNET 0x03

#define SET_FEATURE_COMMAND 0xFD
#define INITIALIZATION 0x04
#define FORCE_FLUSH_SENSOR 0xF0

#define MAX_PACKET_SIZE 512

bool bno085_init();
bool requestID();
void flush();
void softReset();
bool sendPacket(uint8_t channelNumber, uint8_t dataLength, uint8_t *data);
bool receivePacket();
bool checkContinuation();
void clearBuffer(uint8_t *shtpData);
bool enableGyro();
bool parseGyro(uint16_t *gyroAxis);


#endif