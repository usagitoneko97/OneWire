#include <stdint.h>
#include "unity.h"
#include "owcompletesearch.h"
#include "search.h"
#include "mock_onewireio.h"
#include "owvariable.h"
#include <stdlib.h>



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
  eventOw.data = &owdata;

}

void tearDown(void){
  fakeIdBits = NULL;
  fakeCmpIdBits = NULL;

  ((OwData*)(eventOw.data))->uartRxVal = 0;
}


/*void test_owcompletesearch_given_RX_F0_expect_DeviceNA(void)
{
  owRxVal = 0xf0;
  TEST_ASSERT_EQUAL(DEVICE_NA, resetOW());
}*/

/**
 * Expected 1byte return result
 * ---------------------------------
 * 0 1 0 0 1 0 1 1
 *       4b
 * 1 1 1 0 0 0 1 0
 *      e2
 * 0 1 1 0 0 1 1 1
 *       67fi
 */
void test_owcompletesearch_given_OW_presencePulse_RX_10_given_above_number(void){
  uint8_t fakeIdBitVal []=       {0, 1, 0, 0, 0, 1, 1, 1,  0, 1, 0, 1, 0, 0, 1, 0,  0, 1, 0, 0, 0, 1, 1, 0};
  uint8_t fakeCmpIdBitVal[] =   {0, 0, 1, 1, 1, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 1,  0, 0, 0, 1, 1, 0, 0, 1};
  init64BitId(fakeIdBitVal, fakeCmpIdBitVal, 0);

  /*Mocking*/
  setUartBaudRate_Expect(9600);
  //owSetUpRxIT_Expect();
  owUartTxDma_Expect(0xf0);
  /*uart receive it will trigger after uart tx, data will update to below*/
  ((OwData*)(eventOw.data))->uartRxVal = 0xe0;
  //owRxCallBackData = 0xE0;  //data that received in interrupt
  /*Callback from 1 wire receive*/
  isUartFrameError_ExpectAndReturn(FALSE);
  setUartBaudRate_Expect(115200);
  writeSendArray_Expect(sendF0txDataTest, 8);
  //OW_Tx_Expect(sendF0_txData);

  initRomSearching(&eventOw,&owdata);
  eventOw.byteLength = 1;
  owHandler(&eventOw);
  owHandler(&eventOw); //uartRxCallback will call this function

  TEST_ASSERT_EQUAL(0xe2, romDataBuffer[0][0]);
  TEST_ASSERT_EQUAL(0x4b, romDataBuffer[1][0]);
  TEST_ASSERT_EQUAL(TRUE, lastDeviceFlag);
  TEST_ASSERT_EQUAL(0, lastDiscrepancy);

}


void test_owcompletesearch_given_OW_0xf0_expect_noDevice(void){
  /*Mocking*/
  setUartBaudRate_Expect(9600);
  //owSetUpRxIT_Expect();
  owUartTxDma_Expect(0xf0);
  /*uart receive it will trigger after uart tx, data will update to below*/
  ((OwData*)(eventOw.data))->uartRxVal = 0xf0;
  //owRxCallBackData = 0xf0;  //data that received in interrupt
  /*Callback from 1 wire receive*/
  isUartFrameError_ExpectAndReturn(FALSE);

  initRomSearching(&eventOw,&owdata);
  eventOw.byteLength = 1;
  owHandler(&eventOw);
  TEST_ASSERT_EQUAL(FALSE, owHandler(&eventOw)); //callback of uartTx from reset
}

void test_owcompletesearch_given_OW_FrameError_expect_FALSE(void){
  setUartBaudRate_Expect(9600);
  //owSetUpRxIT_Expect();
  owUartTxDma_Expect(0xf0);
  /*uart receive it will trigger after uart tx, data will update to below*/
  ((OwData*)(eventOw.data))->uartRxVal = 0xf0;
  /*Callback from 1 wire receive*/
  isUartFrameError_ExpectAndReturn(TRUE);
  // OW_Rx_ExpectAndReturn(-1);
  initRomSearching(&eventOw,&owdata);
  eventOw.byteLength = 1;
  owHandler(&eventOw);
  TEST_ASSERT_EQUAL(FALSE, owHandler(&eventOw)); //callback of uartTx from reset
}

void test_resetOw_given_state_REPLY_OW_event_UART_FRAME_ERROR_expect_systemError(void){
  /*Mock*/
  Event evt;
  evt.evtType = UART_FRAME_ERROR;
  evt.data = NULL;
  owResetPrivate.state = REPLY_OW;
  systemError_Expect(evt.evtType);

  resetAndVerifyOw(&evt);
}

void test_resetOw_given_state_REPLY_OW_event_UART_TIMEOUT_expect_systemError(void){
  Event evt;
  evt.evtType = UART_TIMEOUT;
  evt.data = NULL;
  owResetPrivate.state = REPLY_OW;
  systemError_Expect(UART_TIMEOUT);

  resetAndVerifyOw(&evt);
}

void test_resetOw_given_state_REPLY_OW_given_uartRxVal_0xe0_event_UART_RX_SUCCESS_expect_DEVICE_AVAILABLE(void){
  Event evt;
  TxRxCpltEvData  txRxCpltEvData;
  *(txRxCpltEvData.uartRxVal) = 0xe0;
  txRxCpltEvData.length = 1;
  evt.evtType = UART_RX_SUCCESS;
  evt.data = &txRxCpltEvData;
  owResetPrivate.state = REPLY_OW;
  TxRxCallbackList txRxNext;
  TxRxCallbackList *txRxListPointer;
  txRxListPointer = &txRxNext;
  txRxListPointer->txRxCallbackFuncP = doRomSearch;
  txRxList.next = txRxListPointer;

  resetAndVerifyOw(&evt);
  TEST_ASSERT_EQUAL(RESET_OW, owResetPrivate.state);

  //TODO test Asserts
}

void test_resetOw_given_state_REPLY_OW_given_uartRxVal_0xf0_event_UART_RX_SUCCESS_expect_DEVICE_NOT_AVAILABLE(void){
  Event evt;
  TxRxCpltEvData  txRxCpltEvData;
  *(txRxCpltEvData.uartRxVal) = 0xf0;
  txRxCpltEvData.length = 1;
  evt.evtType = UART_RX_SUCCESS;
  evt.data = &txRxCpltEvData;
  owResetPrivate.state = REPLY_OW;

  TxRxCallbackList txRxNext;
  TxRxCallbackList *txRxListPointer;
  txRxListPointer = &txRxNext;
  txRxListPointer->txRxCallbackFuncP = doRomSearch;
  txRxList.next = txRxListPointer;

  systemError_Expect(RESET_DEVICE_NOT_AVAILABLE);
  resetAndVerifyOw(&evt);
  TEST_ASSERT_EQUAL(RESET_OW, owResetPrivate.state);
}

void test_resetOw_given_state_REPLY_OW_given_uartRxVal_0xdf_event_UART_RX_SUCCESS_expect_DEVICE_UNKNOWN_ERROR(void){
  Event evt;
  TxRxCpltEvData  txRxCpltEvData;
  *(txRxCpltEvData.uartRxVal) = 0xdf;
  txRxCpltEvData.length = 1;
  evt.evtType = UART_RX_SUCCESS;
  evt.data = &txRxCpltEvData;
  owResetPrivate.state = REPLY_OW;
  TxRxCallbackList txRxNext;
  TxRxCallbackList *txRxListPointer;
  txRxListPointer = &txRxNext;
  txRxListPointer->txRxCallbackFuncP = doRomSearch;
  txRxList.next = txRxListPointer;

  systemError_Expect(RESET_DEVICE_UNKNOWN_ERROR);
  resetAndVerifyOw(&evt);
  TEST_ASSERT_EQUAL(RESET_OW, owResetPrivate.state);
}

void test_romSearching_given_state_SEND_F0_expect_sendf0(void){
  Event evt;
  // evt->evtType =

  romSearchingPrivate.state = SEND_F0;
  uartTxOw_Expect(sendF0txDataTest, 8);
  owSetUpRxIT_Expect(uartRxDataBuffer, 2);
  owUartTxDma_Expect(0xf0);
  romSearching(&evt);
  TEST_ASSERT_EQUAL(ROM_SEARCHING, romSearchingPrivate.state);

}
void test_calcIdCmpId_given_uartRx_0xff_0xfe_expect_idBit1_cmpIdBit0(void){
  uint8_t uartRxVal_[2];
  uartRxVal_[0] = 0xff;
  uartRxVal_[1] = 0xfe;

  int idBitNumber_, cmpIdBitNumber_;
  calcIdCmpId(uartRxVal_, &idBitNumber_, &cmpIdBitNumber_);

  TEST_ASSERT_EQUAL(1, idBitNumber_);
  TEST_ASSERT_EQUAL(0, cmpIdBitNumber_);
}
void test_romSearching_given_state_ROM_SEARCHING_event_UART_RX_SUCCESS_expect_idBitNumber_1_cmpIdBitNumber_0(void){
  TxRxCpltEvData txRxEvData;
  txRxEvData.uartRxVal = malloc(2);
  *(txRxEvData.uartRxVal) = 0xff;
  *(txRxEvData.uartRxVal + 1) = 0xfe;

  romSearchingPrivate.bitNumber = 1;

  Event evt;
  evt.evtType = UART_RX_SUCCESS;
  evt.data = &txRxEvData;
  romSearchingPrivate.state = ROM_SEARCHING;

  romSearching(&evt);
  //TODO
  TEST_ASSERT_EQUAL(1, romSearchingPrivate.idBitNumber);
  TEST_ASSERT_EQUAL(0, romSearchingPrivate.cmpIdBitNumber);
  TEST_ASSERT_EQUAL(2, romSearchingPrivate.bitNumber);
  free(txRxEvData.uartRxVal);
  // evt.data =
  // evt

}
