#include <stdint.h>
#include "unity.h"
#include "owcompletesearch.h"
#include "search.h"
#include "mock_onewireio.h"
#include "owvariable.h"
#include "linkedlist.h"
#include <stdlib.h>
#include "callback.h"


uint8_t owRxVal;
unsigned char bitPos = 0;
uint8_t *fakeIdBits = NULL;
uint8_t *fakeCmpIdBits = NULL;
int fakeReadState = 0;

uint8_t sendF0txDataTest[] = {SEND_ZERO, SEND_ZERO, SEND_ZERO, SEND_ZERO, SEND_ONE, SEND_ONE,SEND_ONE,SEND_ONE};

void init64BitId(uint8_t *id,uint8_t *cmp_id, uint8_t startBit) {
  fakeIdBits = id;
  fakeCmpIdBits = cmp_id;
  bitPos = startBit;
}

uint8_t fakeRead(int numOfCalls){
    uint8_t resultBit;
    if(!lastDeviceFlag){
      while(bitPos < 64){
        switch (fakeReadState) {
          case 0: resultBit = fakeIdBits[bitPos];
                  fakeReadState = 1;
                  return resultBit;
                  break;
          case 1: resultBit = fakeCmpIdBits[bitPos++];
                  fakeReadState = 0;
                  return resultBit;
                  break;
        }
      }
  }
}

/**
 * blank because we didnt need to fake it, we only need to test the event
 */
void fakeOwSendSearchBit(int searchDir, int numOfCalls){
}

void fakeResetBitSearching(BitSearchInformation *bsi, int numOfCalls){
	lastDiscrepancy = bsi->lastZero;        
	if(lastDiscrepancy == 0){              
		lastDeviceFlag = TRUE;                
	}                                       
	clearGet1BitRom(bsi);                   
	bsi->searchResult = TRUE; 
}

void fakeWrite(unsigned char byte, int numOfCalls){
}

void fakewriteSendArray(uint8_t* data, int length, int numOfCalls){

}

uint8_t fake_OW_Rx(int numOfCalls){
  return owRxVal;
}


void setUp(void){
  // OW_Rx_StubWithCallback(fake_OW_Rx);
  Read_StubWithCallback(fakeRead);
  write_StubWithCallback(fakeWrite);
  writeSendArray_StubWithCallback(fakewriteSendArray);
  resetBitSearching_StubWithCallback(fakeResetBitSearching);
  owSendSearchBit_StubWithCallback(fakeOwSendSearchBit);
  eventOw.data = &owdata;

}

void tearDown(void){
  fakeIdBits = NULL;
  fakeCmpIdBits = NULL;

  ((OwData*)(eventOw.data))->uartRxVal = 0;
}

//============================================================================
/*intepretSearchBit function test*/
void test_intepretSearchBit_given_uartRx_0xff_0xfe_expect_BIT_1(void){
  uint8_t uartRxVal_[2];
  uartRxVal_[0] = 0xff;
  uartRxVal_[1] = 0xfe;

  TEST_ASSERT_EQUAL(BIT_1 ,intepretSearchBit(uartRxVal_));
}

void test_intepretSearchBit_given_uartRx_0xfe_0xff_expect_BIT_0(void){
  uint8_t uartRxVal_[2];
  uartRxVal_[0] = 0xfe;
  uartRxVal_[1] = 0xff;

  TEST_ASSERT_EQUAL(BIT_0 ,intepretSearchBit(uartRxVal_));
}

void test_intepretSearchBit_given_uartRx_0xff_0xff_expect_BIT_0(void){
  uint8_t uartRxVal_[2];
  uartRxVal_[0] = 0xff;
  uartRxVal_[1] = 0xff;

  TEST_ASSERT_EQUAL(DEVICE_NOT_THERE ,intepretSearchBit(uartRxVal_));
}

void test_intepretSearchBit_given_uartRx_0xfe_0xfe_expect_BIT_0(void){
  uint8_t uartRxVal_[2];
  uartRxVal_[0] = 0xfe;
  uartRxVal_[1] = 0xfe;

  TEST_ASSERT_EQUAL(BIT_CONFLICT ,intepretSearchBit(uartRxVal_));
}
//============================================================================

/**
 * doRomSearch will initiate a command, and resetAndVerifyOw will initiate a reset
 * pulse.
 */
void test_resetAndVerifyOw_given_state_RESET_OW(void){
  Event eventFromDoRomSearch;
  eventFromDoRomSearch.evtType = INITIATE_COMMAND;
  owResetPrivate.state = RESET_OW;
  owSetUpRxIT_Expect(uartRxDataBuffer, 1);
  owUartTxDma_Expect(0xf0);
  resetAndVerifyOw(&eventFromDoRomSearch);

  Item *itemHead = list.head;
  TxRxCallbackList *headCallbackList = (TxRxCallbackList*)(itemHead->data);
  TEST_ASSERT_EQUAL_PTR(resetAndVerifyOw, headCallbackList->txRxCallbackFuncP);
}

/**
 * uart generate UART_FRAME_ERROR event to resetAndVerifyOw
 */
void test_resetAndVerifyOw_given_state_REPLY_OW_event_UART_FRAME_ERROR_expect_systemError(void){
  /*Mock*/
  Event evt;
  listInit(&list);
  registerCallback(doRomSearch, &list);
  registerCallback(resetAndVerifyOw, &list);

  evt.evtType = UART_FRAME_ERROR;
  evt.data = NULL;
  owResetPrivate.state = REPLY_OW;
  systemError_Expect(evt.evtType);

  resetAndVerifyOw(&evt);
  Item *itemHead = list.head;
  TxRxCallbackList *headCallbackList = (TxRxCallbackList*)(itemHead->data);
  TEST_ASSERT_EQUAL_PTR(doRomSearch, headCallbackList->txRxCallbackFuncP);
  //free(txRxListPointerNext);
}

/**
 * uart timeout Event generated by uartRxCplt callback
 */
void test_resetAndVerifyOw_given_state_REPLY_OW_event_UART_TIMEOUT_expect_systemError(void){
  Event evt;
  evt.evtType = UART_TIMEOUT;
  evt.data = NULL;
  owResetPrivate.state = REPLY_OW;

  listInit(&list);
  registerCallback(doRomSearch, &list);
  registerCallback(resetAndVerifyOw, &list);

  systemError_Expect(UART_TIMEOUT);

  resetAndVerifyOw(&evt);
  Item *itemHead = list.head;
  TxRxCallbackList *headCallbackList = (TxRxCallbackList*)(itemHead->data);
  TEST_ASSERT_EQUAL_PTR(doRomSearch, headCallbackList->txRxCallbackFuncP);
}

/**
 * when resetAndVerifyOw receive that the device is there, will initiate romSearching,
 * where the first task of romSearching is sending f0
 *
 * Expected: registercallback of romSearching
 *           uartTx f0
 *           uart set up rx interrupt 2 bit
 *           uart tx 2 bit of data
 */
void test_resetAndVerifyOw_given_state_REPLY_OW_given_uartRxVal_0xe0_event_UART_RX_SUCCESS_expect_DEVICE_AVAILABLE(void){
  Event evt;
  TxRxCpltEvData  txRxCpltEvData;
  (txRxCpltEvData.uartRxVal) = (uint8_t*)malloc(1);
  *(txRxCpltEvData.uartRxVal) = 0xe0;
  txRxCpltEvData.length = 1;
  evt.evtType = UART_RX_SUCCESS;
  evt.data = &txRxCpltEvData;
  owResetPrivate.state = REPLY_OW;

  listInit(&list);
  registerCallback(doRomSearch, &list);
  registerCallback(resetAndVerifyOw, &list);

  setUartBaudRate_Expect(115200);
  owSetUpRxIT_Expect(uartRxDataBuffer, 10);
  uartTxOw_Expect(sendF0txDataTest, 8);
  owUartTx_Expect(0xff);
  owUartTx_Expect(0xff);

  resetAndVerifyOw(&evt);
  TEST_ASSERT_EQUAL(RESET_OW, owResetPrivate.state);
  Item *itemHead = list.head;
  TxRxCallbackList *headCallbackList = (TxRxCallbackList*)(itemHead->data);
  TEST_ASSERT_EQUAL_PTR(romSearching, headCallbackList->txRxCallbackFuncP);
  free(txRxCpltEvData.uartRxVal);
}

/**
 * Uart successfully receive data, but currently no device in the bus
 */
void test_resetAndVerifyOw_given_state_REPLY_OW_given_uartRxVal_0xf0_event_UART_RX_SUCCESS_expect_DEVICE_NOT_AVAILABLE(void){
  Event evt;
  TxRxCpltEvData  txRxCpltEvData;
  (txRxCpltEvData.uartRxVal) = (uint8_t*)malloc(1);
  *(txRxCpltEvData.uartRxVal) = 0xf0;
  txRxCpltEvData.length = 1;
  evt.evtType = UART_RX_SUCCESS;
  evt.data = &txRxCpltEvData;
  owResetPrivate.state = REPLY_OW;

  listInit(&list);
  registerCallback(doRomSearch, &list);
  /*resetAndVerifyOw initially will register his own self*/
  registerCallback(resetAndVerifyOw, &list);

  systemError_Expect(RESET_DEVICE_NOT_AVAILABLE);
  resetAndVerifyOw(&evt);
  TEST_ASSERT_EQUAL(RESET_OW, owResetPrivate.state);
  free(txRxCpltEvData.uartRxVal);
}

/**
 * uart successfully receive data, but data shows unknown value. (possible device that connected
 * is not 1 wire device)
 */
void test_resetAndVerifyOw_given_state_REPLY_OW_given_uartRxVal_0xdf_event_UART_RX_SUCCESS_expect_DEVICE_UNKNOWN_ERROR(void){
  Event evt;
  TxRxCpltEvData  txRxCpltEvData;
  (txRxCpltEvData.uartRxVal) = (uint8_t*)malloc(1);
  *(txRxCpltEvData.uartRxVal) = 0xdf;
  txRxCpltEvData.length = 1;
  evt.evtType = UART_RX_SUCCESS;
  evt.data = &txRxCpltEvData;
  owResetPrivate.state = REPLY_OW;

  listInit(&list);
  registerCallback(doRomSearch, &list);
  registerCallback(resetAndVerifyOw, &list);

  systemError_Expect(RESET_DEVICE_UNKNOWN_ERROR);
  resetAndVerifyOw(&evt);
  TEST_ASSERT_EQUAL(RESET_OW, owResetPrivate.state);
  free(txRxCpltEvData.uartRxVal);

}

/**
 * given :idBit = 1
 *        cmpIdBit = 1
 * expect :system error ROM_SEARCH_NO_DEVICE
 */
void test_romSearching_error_given_idBit1_cmpIdBit1(void){
  TxRxCpltEvData txRxEvData;
  txRxEvData.uartRxVal = malloc(10);

  //if idBitNumber = 1, it will get the idBit and cmpIdBit at 9th and 10th bit
  //respectively
  *(txRxEvData.uartRxVal + 8) = 0xff;
  *(txRxEvData.uartRxVal + 9) = 0xff;

  listInit(&list);
  registerCallback(doRomSearch, &list);
  registerCallback(romSearching, &list);

  Event evt;
  evt.evtType = UART_RX_SUCCESS;
  evt.data = &txRxEvData;

  BitSearchInformation *bsi = &romSearchingPrivate.bitSearchInformation;
  initGet1BitRom(bsi);  //end of SEND_F0 will call this function
  romSearchingPrivate.bitSearchInformation.idBitNumber = 1; //idBitNumber can be any value

  systemError_Expect(ROM_SEARCH_NO_DEVICE);
  romSearching(&evt);
}
/**
 * NOTE this is the first bit search where the idBitNumber for first search is 1
 */
void test_romSearching_given_state_ROM_SEARCHING_event_UART_RX_SUCCESS_expect_idBitNumber_1_cmpIdBitNumber_0_idBitNumber_1(void){
  TxRxCpltEvData txRxEvData;
  txRxEvData.uartRxVal = malloc(10);
  //if idBitNumber = 1, it will get the idBit and cmpIdBit at 9th and 10th bit
  //respectively
  *(txRxEvData.uartRxVal + 8) = 0xff;
  *(txRxEvData.uartRxVal + 9) = 0xfe;

  Event evt;
  evt.evtType = UART_RX_SUCCESS;
  evt.data = &txRxEvData;

  romSearchingPrivate.state = ROM_SEARCHING;

  BitSearchInformation *bsi = &romSearchingPrivate.bitSearchInformation;
  initGet1BitRom(bsi);  //end of SEND_F0 will call this function
  romSearchingPrivate.bitSearchInformation.idBitNumber = 1;
  owUartTxDma_Expect(0xff);
  owUartTxDma_Expect(0xff);

  romSearching(&evt);
  //TODO
  TEST_ASSERT_EQUAL(2, romSearchingPrivate.bitSearchInformation.idBitNumber);
  TEST_ASSERT_EQUAL(0x01, *(romSearchingPrivate.bitSearchInformation.romUid));
  free(romSearchingPrivate.romUid);
  free(txRxEvData.uartRxVal);
  // evt.data =
  // evt

}

/**
 * testing the romSearching function when rx complete callback pass in 2 values,
 * idBit and cmpIdBit, 0 and 1 respectively, will get the correct result
 */
void test_romSearching_given_state_ROM_SEARCHING_event_UART_RX_SUCCESS_expect_idBitNumber_0_cmpIdBitNumber_1_idBitNumber_8(void){
  owLength = 64;
  TxRxCpltEvData txRxEvData;
  txRxEvData.uartRxVal = malloc(10);
  //if idBitNumber = 1, it will get the idBit and cmpIdBit at 9th and 10th bit
  //respectively
  *(txRxEvData.uartRxVal + 8) = 0xfe;
  *(txRxEvData.uartRxVal + 9) = 0xff;

  Event evt;
  evt.evtType = UART_RX_SUCCESS;
  evt.data = &txRxEvData;

  BitSearchInformation *bsi = &romSearchingPrivate.bitSearchInformation;
  initGet1BitRom(bsi);  //end of SEND_F0 will call this function
  romSearchingPrivate.bitSearchInformation.romByteNum = 0;
  romSearchingPrivate.bitSearchInformation.idBitNumber = 8;
  romSearchingPrivate.bitSearchInformation.byteMask = 128;


  owUartTxDma_Expect(0xff);
  owUartTxDma_Expect(0xff);
  romSearching(&evt);
  //TODO
  TEST_ASSERT_EQUAL(9, romSearchingPrivate.bitSearchInformation.idBitNumber);
  TEST_ASSERT_EQUAL(1, romSearchingPrivate.bitSearchInformation.romByteNum);
  TEST_ASSERT_EQUAL(1, romSearchingPrivate.bitSearchInformation.byteMask);
  TEST_ASSERT_EQUAL(0x0, *(romSearchingPrivate.bitSearchInformation.romUid));
  free(romSearchingPrivate.romUid);
  free(txRxEvData.uartRxVal);

}
/**
 * given: the final bit
 * expect: generate event and execute callback function to parent
 */
void test_romSearching_lastBit(void){
  TxRxCpltEvData txRxEvData;
  txRxEvData.uartRxVal = malloc(3);
  //if idBitNumber != 1, it will get the idBit and cmpIdBit at 2nd and 3rd bit
  //respectively
  *(txRxEvData.uartRxVal + 1) = 0xff;
  *(txRxEvData.uartRxVal + 2) = 0xfe;

  Event evt;
  evt.evtType = UART_RX_SUCCESS;
  evt.data = &txRxEvData;

  BitSearchInformation *bsi = &romSearchingPrivate.bitSearchInformation;
  initGet1BitRom(bsi);  //end of SEND_F0 will call this function
  romSearchingPrivate.bitSearchInformation.romByteNum = 7;
  romSearchingPrivate.bitSearchInformation.idBitNumber = 64; //the last bit
  romSearchingPrivate.bitSearchInformation.byteMask = 0x80;

  listInit(&list);
  registerCallback(doRomSearch, &list);
  registerCallback(romSearching, &list);

  romSearching(&evt);
  TEST_ASSERT_EQUAL(1, romSearchingPrivate.bitSearchInformation.idBitNumber);
  TEST_ASSERT_EQUAL(0, romSearchingPrivate.bitSearchInformation.romByteNum);
  TEST_ASSERT_EQUAL(1, romSearchingPrivate.bitSearchInformation.byteMask);
  TEST_ASSERT_EQUAL(0x80, *(doRomSearchPrivate.romVal + 7));
  free(txRxEvData.uartRxVal);
}

/**
 * test error generated when caller didnt give INITIATE_COMMAND
 */
void test_romSearching_given_state_SEND_F0_UNKNOWN_COMMAND_expect_systemError(void){
  Event unknownEvent;
  unknownEvent.evtType = UNKNOWN_ERROR;
  romSearchingPrivate.state = SEND_F0;

  listInit(&list);
  registerCallback(doRomSearch, &list);
  systemError_Expect(UNKNOWN_ERROR);

  romSearching(&unknownEvent);
  Item *itemHead = list.head;
  TxRxCallbackList *headCallbackList = (TxRxCallbackList*)(itemHead->data);
  TEST_ASSERT_EQUAL_PTR(doRomSearch, headCallbackList->txRxCallbackFuncP);
}

/**
 * final testing on the whole searching process
 */
 void test_doRomSearch_complete_process(){

   //===========================================================
   // clearAllState();
   listInit(&list);

   Event initiateRomSearch;
   initiateRomSearch.evtType = START_ROM_SEARCH;
   owSetUpRxIT_Expect(uartRxDataBuffer, 1);
   owUartTxDma_Expect(0xf0);
   doRomSearch(&initiateRomSearch);
   Item *itemHead = list.head;
   TxRxCallbackList *headCallbackList = (TxRxCallbackList*)(itemHead->data);
   TEST_ASSERT_EQUAL_PTR(resetAndVerifyOw,headCallbackList->txRxCallbackFuncP);
   TxRxCallbackList *nextCallbackList = (TxRxCallbackList*)(itemHead->next->data);
   TEST_ASSERT_EQUAL_PTR(doRomSearch, nextCallbackList->txRxCallbackFuncP);
   TEST_ASSERT_EQUAL(REPLY_OW, owResetPrivate.state);

   //=============================================================
   Event evt;
   TxRxCpltEvData txRxCpltEvData;
   (txRxCpltEvData.uartRxVal) = (uint8_t*)malloc(1);
   *(txRxCpltEvData.uartRxVal) = 0xe0;
   txRxCpltEvData.length = 1;
   evt.evtType = UART_RX_SUCCESS;
   evt.data = &txRxCpltEvData;
   owResetPrivate.state = REPLY_OW;

   setUartBaudRate_Expect(115200);
   owSetUpRxIT_Expect(uartRxDataBuffer, 10);
   uartTxOw_Expect(sendF0txDataTest, 8);
   owUartTx_Expect(0xff);
   owUartTx_Expect(0xff);

   resetAndVerifyOw(&evt);
   Item *itemHead1 = list.head;
   TxRxCallbackList *headCallbackList1 = (TxRxCallbackList*)(itemHead1->data);

   TEST_ASSERT_EQUAL(ROM_SEARCHING, romSearchingPrivate.state);
   TEST_ASSERT_EQUAL_PTR(romSearching, headCallbackList1->txRxCallbackFuncP);
   TEST_ASSERT_EQUAL(RESET_OW, owResetPrivate.state);
   free(txRxCpltEvData.uartRxVal);

   //================================================================
   //Assume data 3 bits = 010  ----------- Device 1.
   //                     011  ----------- Device 2.
   //first interrupt correspond to the first bit [0]
   //first bit of the both devices are conflict, therefore result in
   //uartRxVal = 0xfe, 0xfe
   owLength = 3; //assume size of the rom is 3 bits
   Event romSearchingEv ;
   romSearchingEv.evtType = UART_RX_SUCCESS;

   TxRxCpltEvData txRxEvData;
   txRxEvData.uartRxVal = malloc(10);
   *(txRxEvData.uartRxVal + 8) = 0xfe;
   *(txRxEvData.uartRxVal + 9) = 0xfe;
   romSearchingEv.data = &txRxEvData;
   owUartTxDma_Expect(0xff);
   owUartTxDma_Expect(0xff);
   romSearching(&romSearchingEv);
   TEST_ASSERT_EQUAL(2, romSearchingPrivate.bitSearchInformation.idBitNumber);
   TEST_ASSERT_EQUAL(1, romSearchingPrivate.bitSearchInformation.lastZero);
   TEST_ASSERT_EQUAL(0, romSearchingPrivate.bitSearchInformation.romUid[0]);
   TEST_ASSERT_EQUAL(FALSE, lastDeviceFlag);
   free(txRxEvData.uartRxVal);
   //=====================================================================
   //second run correspond to the second bit [1]
   //second bit both device is 1, there is no conflict and uartRxval is
   //0xff and 0xfe
   romSearchingEv.evtType = UART_RX_SUCCESS;
   txRxEvData.uartRxVal = malloc(3);
   *(txRxEvData.uartRxVal + 1) = 0xff;
   *(txRxEvData.uartRxVal + 2) = 0xfe;
   romSearchingEv.data = &txRxEvData;
   owUartTxDma_Expect(0xff);
   owUartTxDma_Expect(0xff);
   romSearching(&romSearchingEv);
   TEST_ASSERT_EQUAL(3, romSearchingPrivate.bitSearchInformation.idBitNumber);
   TEST_ASSERT_EQUAL(1, romSearchingPrivate.bitSearchInformation.lastZero);
   TEST_ASSERT_EQUAL(2, romSearchingPrivate.bitSearchInformation.romUid[0]);
   TEST_ASSERT_EQUAL(FALSE, romSearchingPrivate.bitSearchInformation.searchResult);
   TEST_ASSERT_EQUAL(FALSE, lastDeviceFlag);
   free(txRxEvData.uartRxVal);

   //=====================================================================
   romSearchingEv.evtType = UART_RX_SUCCESS;
   txRxEvData.uartRxVal = malloc(2);
   *(txRxEvData.uartRxVal + 1) = 0xfe;
   *(txRxEvData.uartRxVal + 2) = 0xff;
   romSearchingEv.data = &txRxEvData;

   romSearching(&romSearchingEv);
   TEST_ASSERT_EQUAL(1, romSearchingPrivate.bitSearchInformation.idBitNumber);
   TEST_ASSERT_EQUAL(0, romSearchingPrivate.bitSearchInformation.lastZero);
   TEST_ASSERT_EQUAL(2, romSearchingPrivate.bitSearchInformation.romUid[0]);
   TEST_ASSERT_EQUAL(2, *(doRomSearchPrivate.romVal));
   TEST_ASSERT_EQUAL(FALSE, lastDeviceFlag);
   //the result (which is 2) is found

 }
