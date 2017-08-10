#include "owcompletesearch.h"
#include "onewireio.h"
#include "search.h"
#include "owvariable.h"
#include <stdio.h>

int result_reset;
uint8_t result ;
uint8_t txdata;
int state;
uint8_t sendF0_txData1[] = {SEND_ZERO, SEND_ZERO, SEND_ZERO, SEND_ZERO, SEND_ONE, SEND_ONE,SEND_ONE,SEND_ONE};
/*deviceAvail resetOW(){
  //uart send F0 9600baud
  //expect receive 0x10 to 0x80
  txdata = 0xF0;
  result =  OW_TxRx(&txdata);
  if(result == 0xf0)
    return DEVICE_NA;
  else  //TODO add additional condition
    return DEVICE_AVAILABLE;
}*/


void initRomSearching(Event* evt, void* owdata){
  evt->commandFunction = romSearch;
  evt->data = owdata;
  evt->eventType = RESET_OW;
  resetOw(evt);
}

void romSearch(Event *evt){
  setUartBaudRate(115200);
  /*Write(0);
  Write(1);
  Write(1);*/
  Write_SendArray(sendF0_txData1, 8);
  if(_firstSearch(1)== FALSE){

  }
  while(LastDeviceFlag != TRUE){
    if(_bitSearch(1) == FALSE){

    }

  }

}

int resetOw(Event *evt){
  switch (evt->eventType) {
    case RESET:
      setUartBaudRate(9600);
      owSetUpRxIT(evt);
      owUartTxDma(0xf0);
      break;
    case REPLY:
      if(isUartFrameError()){
        //Throw()
        return FALSE;
      }
      // data = owRxCallBackData;
      if(((OwData*)(evt->data))->uartRxVal == 0xF0){
        //no device response
        // Throw();
        return FALSE;
      }
      // else if(data >= 0x10 && data <= 0x90){
      /*if the higher bit has response */
      else if ((((OwData*)(evt->data))->uartRxVal & 0xf0) != 0xf){
        //device is there
        return TRUE;
      }
      else{
        //unknown state
        return FALSE;
      }

  }
}
