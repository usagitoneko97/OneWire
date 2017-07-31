#ifndef _ONEWIREIO_H
#define _ONEWIREIO_H

#include <stdint.h>
#include "common.h"
/*Global variables*/



uint8_t Read();
void Write(unsigned char byte);
void Write_SendArray(uint8_t* data, int length);
void OW_UartTx(uint8_t data);
uint8_t OW_UartRx();
int isUartFrameError();

#endif // _ONEWIREIO_H
