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
}

void tearDown(void){
  fake_id_bits = NULL;
  fake_cmp_id_bits = NULL;
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
  OW_UartTx_Expect(0xf0);
  isUartFrameError_ExpectAndReturn(FALSE);
  OW_UartRx_ExpectAndReturn(0x10);
  //OW_Tx_Expect(sendF0_txData);

  completeSearch_OW();
  completeSearch_OW();  //callback of uartTx from reset
  completeSearch_OW();  //callback of uartTx from send_f0
  TEST_ASSERT_EQUAL(0xe2, RomDataBuffer[0][0]);
  TEST_ASSERT_EQUAL(0x4b, RomDataBuffer[1][0]);
  TEST_ASSERT_EQUAL(TRUE, LastDeviceFlag);
  TEST_ASSERT_EQUAL(0, LastDiscrepancy);

}

void test_owcompletesearch_given_OW_0xf0_expect_noDevice(void){
  /*Mocking*/
  OW_UartTx_Expect(0xf0);
  isUartFrameError_ExpectAndReturn(FALSE);
  OW_UartRx_ExpectAndReturn(0xf0);

  clear_OWSm();
  completeSearch_OW();
  TEST_ASSERT_EQUAL(FALSE, completeSearch_OW()); //callback of uartTx from reset
}

void test_owcompletesearch_given_OW_FrameError_expect_FALSE(void){
  OW_UartTx_Expect(0xf0);
  isUartFrameError_ExpectAndReturn(TRUE);
  // OW_Rx_ExpectAndReturn(-1);
  clear_OWSm();
  completeSearch_OW();
  TEST_ASSERT_EQUAL(FALSE, completeSearch_OW()); //callback of uartTx from reset
}
