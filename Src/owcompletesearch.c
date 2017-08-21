#include "owcompletesearch.h"
#include "onewireio.h"
#include "owvariable.h"
#include <stdio.h>
#include "search.h"
#include <stdlib.h>
#include "linkedlist.h"
#include "callback.h"

int result_reset;
uint8_t result ;
uint8_t txdata;
int state;
uint8_t sendF0_txData1[] = {SEND_ZERO, SEND_ZERO, SEND_ZERO, SEND_ZERO, SEND_ONE, SEND_ONE,SEND_ONE,SEND_ONE};


void initRomSearching(EventStruct* evt, void *owdata){
  evt->data = owdata;
  evt->eventType = RESET_OW;
  evt->byteLength = 8;
}



/**
 * perform reset operation by issues a reset pulse and verify presence pulse
 * by 1 wire
 * @param evt event that contain message and event data from parent who called this function
 */
void resetAndVerifyOw(Event *evt){
    uint8_t tempUartRxVal;
    static Event generateResetEv;
    switch (owResetPrivate.state) {
      case RESET_OW:
    	  owResetPrivate.state = REPLY_OW;
          registerCallback(resetAndVerifyOw, &list);
          owSetUpRxIT(uartRxDataBuffer, 1);
          owUartTxDma(0xf0);
          break;
      case REPLY_OW:
        owResetPrivate.state = RESET_OW;
        switch (evt->evtType){
          case UART_FRAME_ERROR:
          case UART_TIMEOUT:
              CREATE_EVENT_WITH_TYPE(generateResetEv, evt->evtType);
              unregisterCallback(&list);
              FuncP functPToCaller;
              functPToCaller = getCurrentCallback((&list));
              functPToCaller(&(generateResetEv));
              break;
          case UART_RX_SUCCESS:
              tempUartRxVal = GET_UART_RX_VAL(evt);
              if(OW_DEVICE_NOT_READY(tempUartRxVal)){
                CREATE_EVENT_WITH_TYPE(generateResetEv, RESET_DEVICE_NOT_AVAILABLE);
                unregisterCallback(&list);
                FuncP functPToCaller;
                functPToCaller = getCurrentCallback((&list));
                functPToCaller(&(generateResetEv));
            	}
            	// else if(data >= 0x10 && data <= 0x90){
            	/*if the higher bit has response */
              else if (OW_DEVICE_READY(tempUartRxVal)){
                CREATE_EVENT_WITH_TYPE(generateResetEv, RESET_DEVICE_AVAILABLE);
                unregisterCallback(&list);
                FuncP functPToCaller;
                functPToCaller = getCurrentCallback((&list));
                functPToCaller(&(generateResetEv));
            	}
            	else{
                CREATE_EVENT_WITH_TYPE(generateResetEv, RESET_DEVICE_UNKNOWN_ERROR);
                unregisterCallback(&list);
                FuncP functPToCaller;
                functPToCaller = getCurrentCallback((&list));
                functPToCaller(&(generateResetEv));
            	}
              break;
          }
          break;
    }
}

/**
 * perform rom searching operation to get the rom number of devices in the bus
 * @param evt variables in the type of event that trigger romSearching
 */
void romSearching(Event *evt){
  static Event generateFailEvt;
  BitSearchInformation *bsi = &romSearchingPrivate.bitSearchInformation;
  TxRxCpltEvData *evData = (TxRxCpltEvData*)(evt->data);
  switch (romSearchingPrivate.state) {
    case SEND_F0:
        if(evt->evtType == INITIATE_COMMAND){
          registerCallback(romSearching, &list);
          initGet1BitRom(bsi);
          romSearchingPrivate.state = ROM_SEARCHING;
          uartTxOw(sendF0_txData1, 8);
          owSetUpRxIT(uartRxDataBuffer, 2);
          owUartTx(0xff);
          owUartTx(0xff);
        }
        else {
          generateFailEvt.evtType = UNKNOWN_ERROR;
          //TODO check for null
          FuncP functPToCaller;
          functPToCaller = getCurrentCallback((&list));
          functPToCaller(&(generateFailEvt));
        }
        break;
    case ROM_SEARCHING:
      switch (evt->evtType) {
        case UART_RX_SUCCESS:
              bsi->bitReadType = intepretSearchBit(evData->uartRxVal);
              //TODO check for lastDeviceFlag
              get1BitRom(bsi);
              if(ERROR_NO_DEVICE(romSearchingPrivate)){
                //ERROR
                clearGetRom(&romSearchingPrivate);
                free(romSearchingPrivate.romUid);
                CREATE_EVENT_WITH_TYPE(generateFailEvt, ROM_SEARCH_NO_DEVICE);
                //TODO check for null
                unregisterCallback(&list);
                FuncP functPToCaller;
                functPToCaller = getCurrentCallback((&list));
                functPToCaller(&(generateFailEvt));
              }
              else{
                if(SEARCH_COMPLETE(romSearchingPrivate)){
                  static Event generateEvt;
                  CREATE_EVENT_WITH_TYPE(generateEvt, ROM_SEARCH_SUCCESSFUL);
                  static RomSearchingEvData evData;
                  evData.romDataBuffer = romSearchingPrivate.bitSearchInformation.romUid;
                  evData.lastDeviceFlag = lastDeviceFlag;
                  generateEvt.data = &evData;
                  clearGetRom(&romSearchingPrivate);
                  unregisterCallback(&list);
                  FuncP functPToCaller;
                  functPToCaller = getCurrentCallback((&list));
                  functPToCaller(&(generateEvt));
                }
                else{
                  owSetUpRxIT(uartRxDataBuffer, 2);
                  owUartTxDma(0xff);
                  owUartTxDma(0xff);
                }
              }
            break;
        case UART_FRAME_ERROR:
        case UART_TIMEOUT:
            CREATE_EVENT_WITH_TYPE(generateFailEvt, evt->evtType);
            FuncP functPToCaller;
            functPToCaller = getCurrentCallback((&list));
            functPToCaller(&(generateFailEvt));
            //TODO
            //free romUid
            break;

          //TODO free romSearchingPrivate.romUid
      }
      break;
  }
}

/**
 * analyze the received value from uart and return a SearchBitType
 * @param  uartRxVal pointer of uint8_t that contains received value from uart
 * @return           the information after intepret idBits and cmpIdbit in SearchBitType
 */
SearchBitType intepretSearchBit(uint8_t *uartRxVal){
  if((*uartRxVal == 0xff) && (*(uartRxVal+1) == 0xff)){
    return DEVICE_NOT_THERE;
  }
  else if((*uartRxVal != 0xff) && (*(uartRxVal+1) == 0xff)){
    return BIT_0;
  }
  else if ((*uartRxVal == 0xff) && (*(uartRxVal+1) != 0xff)){
    return BIT_1;
  }
  else{
    return BIT_CONFLICT;
  }
}


/**
 * initialize search operation
 * @param bsi BitSearchInformation datatype
 */
void initGet1BitRom(BitSearchInformation *bsi){
  bsi->lastZero = 0;
  bsi->romByteNum = 0;
  bsi->byteMask = 1;
  bsi->searchResult = 0;
  bsi->noDevice = FALSE;
  bsi->idBitNumber = 1;

  clearDataBuffer64();
  lastDiscrepancy = 0;
  lastDeviceFlag=FALSE;
  lastFamilyDiscrepancy = 0;
  //TODO change the state somewhere
    // romSearchingPrivate->state = ROM_SEARCHING;
  bsi->romUid = malloc(8);
  *(bsi->romUid) = 0;
}


/**
 * function to perform romSearch (parent to romSearching and resetAndVerifyOw)
 * @param evt [description]
 */
void doRomSearch(Event *evt){
  uint8_t *dataTemp;
  Event doRomSearchEv;
  switch (evt->evtType) {
    //event of START_ROM_SEARCH will be initialize by main
    case START_ROM_SEARCH:
      registerCallback(doRomSearch, &list);
      resetAndVerifyOw(&doRomSearchEv);
      volatile FuncP fp;
      fp = getCurrentCallback(&list);
      volatile int i = 0;
      i++;
      break;
    //resetAndVerifyOw successfully detect 1 wire device
    case RESET_DEVICE_AVAILABLE:
      doRomSearchEv.evtType = INITIATE_COMMAND;
      setUartBaudRate(115200);
      romSearching(&doRomSearchEv);
      break;
    //romSearching successfully searched rom
    case ROM_SEARCH_SUCCESSFUL:
      doRomSearchPrivate.romVal = ((RomSearchingEvData*)(evt->data))->romDataBuffer;
      break;
    case ROM_SEARCH_NO_DEVICE:
    case UART_TIMEOUT:
    case UART_FRAME_ERROR:
    case RESET_DEVICE_NOT_AVAILABLE:
    case RESET_DEVICE_UNKNOWN_ERROR:
    case UNKNOWN_ERROR:
      systemError(evt->evtType);
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
  //free(romSearchingPrivate->romUid);
}


int initConvertT(){
  //TODO implement initConvertT command
  return 0;
}
