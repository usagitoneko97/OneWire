#include <stdint.h>
#include "unity.h"
#include "owcompletesearch.h"
#include "search.h"
#include "mock_onewireio.h"
#include "owvariable.h"



uint8_t OW_Rx_val;
unsigned char bitPos = 0;
uint8_t *fake_id_bits = NULL;
uint8_t *fake_cmp_id_bits = NULL;
int state_fakeRead = 0;

uint8_t sendF0_txData_test[] = {SEND_ZERO, SEND_ZERO, SEND_ZERO, SEND_ZERO, SEND_ONE, SEND_ONE,SEND_ONE,SEND_ONE};

void init64BitId(uint8_t *id,uint8_t *cmp_id, uint8_t startBit) {
  fake_id_bits = id;
  fake_cmp_id_bits = cmp_id;
  bitPos = startBit;
}

uint8_t fake_Read(int numOfCalls){
    uint8_t result_bit;
    if(!LastDeviceFlag){
      while(bitPos < 64){
        switch (state_fakeRead) {
          case 0: result_bit = fake_id_bits[bitPos];
                  state_fakeRead = 1;
                  return result_bit;
                  break;
          case 1: result_bit = fake_cmp_id_bits[bitPos++];
                  state_fakeRead = 0;
                  return result_bit;
                  break;
        }
      }
  }
}

void fake_Write(unsigned char byte, int numOfCalls){
}

void fake_Write_SendArray(uint8_t* data, int length, int numOfCalls){

}

uint8_t fake_OW_Rx(int numOfCalls){
  return OW_Rx_val;
}


void setUp(void){
  // OW_Rx_StubWithCallback(fake_OW_Rx);
  Read_StubWithCallback(fake_Read);
  Write_StubWithCallback(fake_Write);
  Write_SendArray_StubWithCallback(fake_Write_SendArray);
  eventOw.data = &owdata;
}

void tearDown(void){
  fake_id_bits = NULL;
  fake_cmp_id_bits = NULL;

  ((OwData*)(eventOw.data))->uartRxVal = 0;
}


/*void test_owcompletesearch_given_RX_F0_expect_DeviceNA(void)
{
  OW_Rx_val = 0xf0;
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
  uint8_t fake_id_bit_VAL []=       {0, 1, 0, 0, 0, 1, 1, 1,  0, 1, 0, 1, 0, 0, 1, 0,  0, 1, 0, 0, 0, 1, 1, 0};
  uint8_t fake_cmp_id_bit_VAL[] =   {0, 0, 1, 1, 1, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 1,  0, 0, 0, 1, 1, 0, 0, 1};
  init64BitId(fake_id_bit_VAL, fake_cmp_id_bit_VAL, 0);
  /*Mocking*/
  setUartBaudRate_Expect(9600);
  owSetUpRxIT_Expect();
  owUartTxDma_Expect(0xf0);
  /*uart receive it will trigger after uart tx, data will update to below*/
  ((OwData*)(eventOw.data))->uartRxVal = 0xe0;
  //owRxCallBackData = 0xE0;  //data that received in interrupt
  /*Callback from 1 wire receive*/
  isUartFrameError_ExpectAndReturn(FALSE);
  setUartBaudRate_Expect(115200);
  Write_SendArray_Expect(sendF0_txData_test, 8);
  //OW_Tx_Expect(sendF0_txData);

  initRomSearching(&eventOw, &owdata);
  eventOw.eventType = REPLY;
  resetOw(&eventOw);    //uartRxCallback will check the reply
  romSearch(&eventOw);  //uartRxCallback will call this function
  TEST_ASSERT_EQUAL(0xe2, RomDataBuffer[0][0]);
  TEST_ASSERT_EQUAL(0x4b, RomDataBuffer[1][0]);
  TEST_ASSERT_EQUAL(TRUE, LastDeviceFlag);
  TEST_ASSERT_EQUAL(0, LastDiscrepancy);

}


void test_owcompletesearch_given_OW_0xf0_expect_noDevice(void){
  /*Mocking*/
  setUartBaudRate_Expect(9600);
  owSetUpRxIT_Expect();
  owUartTxDma_Expect(0xf0);
  /*uart receive it will trigger after uart tx, data will update to below*/
  ((OwData*)(eventOw.data))->uartRxVal = 0xf0;
  //owRxCallBackData = 0xf0;  //data that received in interrupt
  /*Callback from 1 wire receive*/
  isUartFrameError_ExpectAndReturn(FALSE);

  initRomSearching(&eventOw, &owdata);
  eventOw.eventType = REPLY;
  TEST_ASSERT_EQUAL(FALSE, resetOw(&eventOw)); //callback of uartTx from reset
}

void test_owcompletesearch_given_OW_FrameError_expect_FALSE(void){
  setUartBaudRate_Expect(9600);
  owSetUpRxIT_Expect();
  owUartTxDma_Expect(0xf0);
  /*uart receive it will trigger after uart tx, data will update to below*/
  ((OwData*)(eventOw.data))->uartRxVal = 0xf0;
  /*Callback from 1 wire receive*/
  isUartFrameError_ExpectAndReturn(TRUE);
  // OW_Rx_ExpectAndReturn(-1);
  initRomSearching(&eventOw, &owdata);
  eventOw.eventType = REPLY;
  TEST_ASSERT_EQUAL(FALSE, resetOw(&eventOw)); //callback of uartTx from reset
}
