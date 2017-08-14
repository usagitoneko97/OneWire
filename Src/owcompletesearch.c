#include "owcompletesearch.h"
#include "onewireio.h"
#include "owvariable.h"
#include <stdio.h>
#include "search.h"
#include <stdlib.h>
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
    //owSetUpRxIT(evt);
    owUartTxDma(0xf0);
}


void resetAndVerifyOw(Event *evt){
    uint8_t tempUartRxVal;
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
            //FIXME funcPointer go to parent, parent will call systemError
            generateResetEv.evtType = evt->evtType;
            txRxList.next->txRxCallbackFuncP(&generateResetEv);
            break;
          case UART_RX_SUCCESS:
            //checking..

            tempUartRxVal = *(((TxRxCpltEvData*)evt->data)->uartRxVal);
            if(tempUartRxVal == 0xF0){
              generateResetEv.evtType = RESET_DEVICE_NOT_AVAILABLE;
              txRxList.next->txRxCallbackFuncP(&generateResetEv);
          	}
          	// else if(data >= 0x10 && data <= 0x90){
          	/*if the higher bit has response */
            else if ((tempUartRxVal & 0x0f) == 0x0 && (tempUartRxVal & 0xf0) != 0xf0){
              generateResetEv.evtType = RESET_DEVICE_AVAILABLE;
              txRxList.next->txRxCallbackFuncP(&generateResetEv);
          	}
          	else{
              generateResetEv.evtType = RESET_DEVICE_UNKNOWN_ERROR;
              txRxList.next->txRxCallbackFuncP(&generateResetEv);
          	}
            break;
          }
          break;
    }
}


void romSearching(Event *evt){
  static Event generateFailEvt;
  BitSearchInformation *bsi = &romSearchingPrivate.bitSearchInformation;
  TxRxCpltEvData *evData = (TxRxCpltEvData*)(evt->data);
  switch (romSearchingPrivate.state) {
    case SEND_F0:
      initGetBitRom(&romSearchingPrivate);
      uartTxOw(sendF0_txData1, 8);
      owSetUpRxIT(uartRxDataBuffer, 2);
      owUartTxDma(0xf0);

      break;
    case ROM_SEARCHING:
      switch (evt->evtType) {
        case UART_RX_SUCCESS:
            calcIdCmpId(evData->uartRxVal, &bsi->idBit, &bsi->cmpIdBit);
            //TODO check for lastDeviceFlag
            get1BitRom(&romSearchingPrivate);
            if(romSearchingPrivate.bitSearchInformation.noDevice == TRUE){
              //ERROR
              clearGetRom(&romSearchingPrivate);
              free(romSearchingPrivate.romNo);
              generateFailEvt.evtType = ROM_SEARCH_NO_DEVICE;
              //TODO check for null
              // ((TxRxCallbackList*)(((list.head)->next)->data))->txRxCallbackFuncP(&generateFailEvt);
              (txRxList.next)->txRxCallbackFuncP(&generateFailEvt);
            }
            else{
              if(romSearchingPrivate.bitSearchInformation.idBitNumber > 63){
                updateSearch(&romSearchingPrivate);
                static Event generateEvt;
                generateEvt.evtType = ROM_SEARCH_SUCCESSFUL;
                static RomSearchingEvData evData;
                evData.romDataBuffer = romSearchingPrivate.romNo;
                evData.lastDeviceFlag = lastDeviceFlag;
                generateEvt.data = &evData;
                clearGetRom(&romSearchingPrivate);
                (txRxList.next)->txRxCallbackFuncP(&generateEvt); //go back to parent
              }
              else{
                owSetUpRxIT(uartRxDataBuffer, 2);
                owUartTxDma(0xf0);
              }
            }
          break;
        case UART_FRAME_ERROR:
        case UART_TIMEOUT:
          generateFailEvt.evtType = evt->evtType;
          txRxList.next->txRxCallbackFuncP(&generateFailEvt);
          //TODO
          //free romNo
          break;

          //TODO free romSearchingPrivate.romNo
      }
      break;
  }
}

void updateSearch(RomSearchingPrivate *romSearchingPrivate){
  lastDiscrepancy = (romSearchingPrivate->bitSearchInformation).lastZero;
  if(lastDiscrepancy == 0){
    lastDeviceFlag = TRUE;
  }
  (romSearchingPrivate->bitSearchInformation).searchResult = TRUE;
}

void calcIdCmpId(uint8_t *uartRxVal, int *idBitNumber, int *cmpIdBitNumber){
  if(*(uartRxVal) == 0xff){
    *idBitNumber = 1;
  }
  else{
    *idBitNumber = 0;
  }
  if(*(uartRxVal+1) == 0xff){
    *cmpIdBitNumber = 1;
  }
  else{
    *cmpIdBitNumber = 0;
  }

}



void initGetBitRom(RomSearchingPrivate *romSearchingPrivate){
  (romSearchingPrivate->bitSearchInformation).lastZero = 0;
  (romSearchingPrivate->bitSearchInformation).romByteNum = 0;
  (romSearchingPrivate->bitSearchInformation).byteMask = 1;
  (romSearchingPrivate->bitSearchInformation).searchResult = 0;
  (romSearchingPrivate->bitSearchInformation).noDevice = FALSE;

  romSearchingPrivate->state = ROM_SEARCHING;
  romSearchingPrivate->romNo = malloc(8);
  *(romSearchingPrivate->romNo) = 0;
  //move txRxlist to next
  //insert romSearching at head
  txRxList.txRxCallbackFuncP = romSearching;
}

void doRomSearch(Event *evt){
  uint8_t *dataTemp;
  switch (evt->evtType) {
    case RESET_DEVICE_AVAILABLE:
      printf("reset device available\n");
      static Event romSearchEv;
      //TODO go tom romSearching
      break;
    case ROM_SEARCH_SUCCESSFUL:
      //generate event to sent to parent
      // doRomSearchPrivate.romVal = malloc(2);
      doRomSearchPrivate.romVal = ((RomSearchingEvData*)(evt->data))->romDataBuffer;
      printf("Rom Search success!\n");
      break;
    case ROM_SEARCH_NO_DEVICE:
      systemError(evt->evtType);
      printf("Rom search no device\n");
      break;
    case UART_TIMEOUT:
    case UART_FRAME_ERROR:
    case RESET_DEVICE_NOT_AVAILABLE:
    case RESET_DEVICE_UNKNOWN_ERROR:
    // printf("reset device error\n");
    systemError(evt->evtType);
    // owSetUpRxIT(uartRxDataBuffer, 2);
    // dummy();
    break;
  }
}

void clearGetRom(RomSearchingPrivate *romSearchingPrivate){
  romSearchingPrivate->state = SEND_F0;
  (romSearchingPrivate->bitSearchInformation).lastZero = 0;
  (romSearchingPrivate->bitSearchInformation).romByteNum = 0;
  (romSearchingPrivate->bitSearchInformation).byteMask = 1;
  (romSearchingPrivate->bitSearchInformation).searchResult = 0;
  (romSearchingPrivate->bitSearchInformation).noDevice = FALSE;
  (romSearchingPrivate->bitSearchInformation).idBitNumber = 1;
  //TODO find a way to free it
  //free(romSearchingPrivate->romNo);
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
