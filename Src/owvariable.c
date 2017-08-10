#include "owvariable.h"
unsigned char romNo[8];
unsigned char RomDataBuffer[MAX_OWDEVICE][8];
int bufferByteNumber;
int bufferDeviceNumber;
int lastDiscrepancy;
int LastFamilyDiscrepancy;
int LastDeviceFlag;
unsigned char crc8;
Event eventOw;
OwData owdata;
uint8_t owRxCallBackData;
