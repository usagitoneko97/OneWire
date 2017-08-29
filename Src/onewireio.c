#include "onewireio.h"

void owSendSearchBit(int searchDir){
  owSetUpRxIT(uartRxDataBuffer, 3);
  write(searchDir);
}

void resetBitSearching(BitSearchInformation *bsi){
	lastDiscrepancy = bsi->lastZero;        
	if(lastDiscrepancy == 0){              
		lastDeviceFlag = TRUE;                
	}                                       
	clearGet1BitRom(bsi);                   
	bsi->searchResult = TRUE;               
}