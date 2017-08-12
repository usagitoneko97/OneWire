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


void initRomSearching(EventStruct* evt, void* owdata){
  evt->commandFunction = romSearch;
  evt->data = owdata;
  evt->eventType = RESET_OW;
  evt->byteLength = 8;
}

void romSearch(EventStruct *evt){
  setUartBaudRate(115200);
  /*write(0);
  write(1);
  write(1);*/
  writeSendArray(sendF0_txData1, 8);
  if(_firstSearch(evt->byteLength) == FALSE){

  }
  while(lastDeviceFlag != TRUE){
    if(_bitSearch(evt->byteLength) == FALSE){

    }

  }
  	volatile int i;
  	i++;

}

void resetOw(EventStruct *evt){
    setUartBaudRate(9600);
    owSetUpRxIT(evt);
    owUartTxDma(0xf0);
}


void resetAndVerifyOw(Event *evt){
    static Event generateResetEv;
    switch (owResetPrivate.state) {
      case RESET_OW:
        //register callback
        break;
      case REPLY_OW:
        owResetPrivate.state = RESET_OW;
        switch (evt->evtType){
          case UART_FRAME_ERROR:
          case UART_TIMEOUT:
            systemError(evt->evtType);
            break;
          case UART_RX_SUCCESS:
            generateResetEv.evtType = RESET_DEVICE_AVAILABLE;
            txRxList.next->txRxCallbackFuncP(&generateResetEv);
            break;
        }
        break;
    }
}

void doRomSearch(Event *evt){
  printf("successful%s\n");
}

int initConvertT(){
  return 0;
}

int isOwDeviceAvail(EventStruct *evt){
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

int owHandler(EventStruct *evt){
	switch(evt->eventType){
	case RESET:
		evt->eventType = REPLY;
		resetOw(evt);
		break;
	case REPLY:
		evt->eventType = RESET;
		if(isOwDeviceAvail(evt)){
			evt->commandFunction(&eventOw);
		}
		else{
			return FALSE;
		}
		break;
	default: return FALSE;
	}
	return FALSE;
}
