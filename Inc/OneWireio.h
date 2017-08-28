#ifndef _ONEWIREIO_H
#define _ONEWIREIO_H

#include <stdint.h>
#include "common.h"
#include "OwCompleteSearch.h"
#include "OwVariable.h"

#ifndef TEST
  #define OW_LENGTH 64
#else
  #define OW_LENGTH  owLength
#endif

/*Global variables*/



uint8_t Read();
void write(unsigned char byte);
void writeSendArray(uint8_t* data, int length);
void owSetUpRxIT(uint8_t* data, int bitLength);
void owUartTxDma(uint8_t data);
void owUartTx(uint8_t data);
uint8_t owUartRx();
int isUartFrameError();
void setUartBaudRate(int baudRate);
void resetUart(int baudRate);
void uartDelay(int delay);

void uartTxOw(uint8_t* data, int bitLength);

void systemError(EventType evtType);

void dummy();
#endif // _ONEWIREIO_H
