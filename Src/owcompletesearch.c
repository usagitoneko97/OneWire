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

void clear_OWSm(){
  state = 0;
}


int search_SM(Event* event){
  uint8_t data;
  switch (event->eventType) {
    case RESET_OW:
    	  owSetUpRxIT(event);
    	  owUartTxDma(0xf0);
          return TRUE;

    case REPLY:	//rxIt trigger here
          if(isUartFrameError()){
            //Throw()
            return FALSE;
          }
          // data = owRxCallBackData;
          if(((OwData*)(event->data))->uartRxVal == 0xF0){
            //no device response
            // Throw();
            return FALSE;
          }
          // else if(data >= 0x10 && data <= 0x90){
          /*if the higher bit has response */
          else if ((((OwData*)(event->data))->uartRxVal & 0xf0) != 0xf){
            //device is there
            return TRUE;
          }
          else{
            //unknown state
            return FALSE;
          }

    case SEND_F0:
          Write_SendArray(sendF0_txData1, 8);
          return TRUE;

    case BITSEARCH:
          if(_firstSearch(1)== FALSE){
            return FALSE;
          }
          while(LastDeviceFlag != TRUE){
            if(_bitSearch(1) == FALSE)
              return FALSE;
          }
    	  /*if(firstSearch())
    		  return TRUE;
    	  else
    		  return FALSE;*/
    default:
    	// dump system error
    	return FALSE;
  }
}

int completeSearch_OW(){

  switch (state) {
    case RESET:
    	 setUartBaudRate(9600);
    	  state = 1;  //assume that this fuc will be uart_tx callback
        eventOw.eventType = RESET_OW;
        search_SM(&eventOw);
        return TRUE;
    case 1:
        eventOw.eventType = REPLY;
        if(search_SM(&eventOw)){  //return true if 1 wire device is detected
        	volatile int i;

        	i++;
        	setUartBaudRate(115200);
        	/*Write(0);
        	Write(1);
        	Write(1);*/
          eventOw.eventType = SEND_F0;
        	search_SM(&eventOw);
        	i++;

        	//state = 2;  //assume that this fuc will be uart_tx callback
        	//return TRUE; //dont return to go to next case
        	eventOw.eventType = BITSEARCH;
        	if(search_SM(&eventOw)){
        		//success
        		state = RESET;
        		return TRUE;
        	}
        	else{
        		// throw();
        		state = RESET;
        		return FALSE; //process done
        	}
        	}
          //throw();
          return FALSE;


    default:
        state = RESET;
      	return FALSE;
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
      if(((OwData*)(event->data))->uartRxVal == 0xF0){
        //no device response
        // Throw();
        return FALSE;
      }
      // else if(data >= 0x10 && data <= 0x90){
      /*if the higher bit has response */
      else if ((((OwData*)(event->data))->uartRxVal & 0xf0) != 0xf){
        //device is there
        return TRUE;
      }
      else{
        //unknown state
        return FALSE;
      }

  }
}

void romSearch(Event *evt){
  
}
