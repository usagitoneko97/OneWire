#include <stdint.h>
#include "owcompletesearch.h"
#include "linkedlist.h"
#ifndef _OWVARIABLE_H
#define _OWVARIABLE_H
#define MAX_OWDEVICE  64
extern unsigned char romUid[];
extern unsigned char romDataBuffer[][8];
extern int bufferByteNumber;
extern int bufferDeviceNumber;
extern int lastDiscrepancy;
extern int lastFamilyDiscrepancy;
extern int lastDeviceFlag;
extern unsigned char crc8;
extern uint8_t owRxCallBackData;
extern EventStruct eventOw;
extern OwData owdata;
extern OwResetPrivate owResetPrivate;

extern TxRxCallbackList txRxList;
extern LinkedList list;

extern uint8_t *uartRxDataBuffer;
extern RomSearchingPrivate romSearchingPrivate;

extern DoRomSearchPrivate doRomSearchPrivate;
extern int owLength;
extern int searchCpltF;
#endif // _OWVARIABLE_H
