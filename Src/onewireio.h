#ifndef _ONEWIREIO_H
#define _ONEWIREIO_H

#include <stdint.h>
#include "common.h"
#include "owcompletesearch.h"
/*Global variables*/



uint8_t Read();
void write(unsigned char byte);
void writeSendArray(uint8_t* data, int length);
void owSetUpRxIT();
void owUartTxDma(uint8_t data);
void owUartTx(uint8_t data);
uint8_t owUartRx();
int isUartFrameError();
void setUartBaudRate(int baudRate);
void resetUart(int baudRate);
void uartDelay(int delay);

void systemError(EventType evtType);
#endif // _ONEWIREIO_H
