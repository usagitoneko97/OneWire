#include <stdint.h>
#include "owcompletesearch.h"
#ifndef _OWVARIABLE_H
#define _OWVARIABLE_H
#define MAX_OWDEVICE  64
extern unsigned char romNo[];
extern unsigned char RomDataBuffer[][8];
extern int bufferByteNumber;
extern int bufferDeviceNumber;
extern int lastDiscrepancy;
extern int LastFamilyDiscrepancy;
extern int LastDeviceFlag;
extern unsigned char crc8;
extern uint8_t owRxCallBackData;
extern Event eventOw;
extern OwData owdata;

#endif // _OWVARIABLE_H
