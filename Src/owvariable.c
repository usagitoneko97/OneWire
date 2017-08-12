#include "owvariable.h"
unsigned char romNo[8];
unsigned char romDataBuffer[MAX_OWDEVICE][8];
int bufferByteNumber;
int bufferDeviceNumber;
int lastDiscrepancy;
int lastFamilyDiscrepancy;
int lastDeviceFlag;
unsigned char crc8;
EventStruct eventOw;
OwData owdata;
uint8_t owRxCallBackData;

OwResetPrivate owResetPrivate;
