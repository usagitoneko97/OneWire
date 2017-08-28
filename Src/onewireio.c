#include "onewireio.h"

void owSendSearchBit(int searchDir){
  owSetUpRxIT(uartRxDataBuffer, 3);
  write(searchDir);
}
