#include "unity.h"
#include "search.h"
#include "mock_onewireio.h"
#include "owvariable.h"
#include "common.h"

// #define NO_OF_DEVICE  4

unsigned char bitPos = 0x01;
int state = 0;
uint8_t *fakeIdBits = NULL;
uint8_t *fakeCmpIdBits = NULL;

#define getBytePos(x)    ((x) >> 3)
#define getBitPos(x)     ((x) & 0x7)

void init64BitId(uint8_t *id,uint8_t *cmp_id, uint8_t startBit) {
  fakeIdBits = id;
  fakeCmpIdBits = cmp_id;
  bitPos = startBit;
}

uint8_t fakeRead(int numOfCalls){
    uint8_t resultBit;
    if(!lastDeviceFlag){
      while(bitPos < 64){
        switch (state) {
          case 0: resultBit = fakeIdBits[bitPos];
                  state = 1;
                  return resultBit;
                  break;
          case 1: resultBit = fakeCmpIdBits[bitPos++];
                  state = 0;
                  return resultBit;
                  break;
        }
      }
  }


}

void fakeWrite(unsigned char byte, int numOfCalls){
  printf("OW Write function is being called..\n");
}


void setUp(void)
{
  Read_StubWithCallback(fakeRead);
  write_StubWithCallback(fakeWrite);
}

void tearDown(void) {
  fakeIdBits = NULL;
  fakeCmpIdBits = NULL;
}

InnerVAR_OW initSearchTest(InnerVAR_OW innerVAR_OW){
  lastDiscrepancy = 0;
  lastDeviceFlag=FALSE;
  lastFamilyDiscrepancy = 0;
  int i = 0;
  while(i<8){
    romNo[i++] = 0;
  }
  innerVAR_OW.idBitNumber = 1;
  innerVAR_OW.lastZero = 0;
  innerVAR_OW.romByteNum = 0;
  innerVAR_OW.rom_byte_mask = 1;
  innerVAR_OW.searchResult = 0;
  innerVAR_OW.idBit = -1;
  innerVAR_OW.cmpIdBit = -1;
  innerVAR_OW.searchDirection = 0;
  return innerVAR_OW;
}
/*Testing return value of fake_read is correct*/
void test_fake_read_return_idBit_cmpIdBit(void){
//  char id[]
  uint8_t fakeIdBitVal []=       {1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1};
  uint8_t fakeCmpIdBitVal[] =   {0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 1};
  init64BitId(fakeIdBitVal, fakeCmpIdBitVal, 0);

  TEST_ASSERT_EQUAL(1, fakeIdBits[0]);
  TEST_ASSERT_EQUAL(0, fakeCmpIdBits[0]);
  lastDeviceFlag = FALSE;
  TEST_ASSERT_EQUAL(1, fakeRead(0));
  TEST_ASSERT_EQUAL(0, fakeRead(0));

  TEST_ASSERT_EQUAL(1, fakeRead(0));
  TEST_ASSERT_EQUAL(1, fakeRead(0));

  TEST_ASSERT_EQUAL(0, fakeRead(0));
  TEST_ASSERT_EQUAL(1, fakeRead(0));
}

/********************************************************
 * Id_bit     = 0                                       *
 * cmpIdBit = 0                                       *
 *                                                      *
 * EXPECTED:                                            *
 * lastZero = 1                                        *
 * idBitNumber ++                                     *
 * searchDirection = idBit                            *
 * first bit of first byte of romNo = searchDirection *
 ********************************************************/
void test_processOWData_IdBit_cmpBit_00(void){
  /*initialize test*/
  uint8_t fakeIdBitVal []=       {0};
  uint8_t fakeCmpIdBitVal[] =   {0};
  init64BitId(fakeIdBitVal, fakeCmpIdBitVal, 0);

  InnerVAR_OW innerVAR_OW;
  innerVAR_OW = initSearchTest(innerVAR_OW);
  /*initialize condition for test*/
  init64BitId(fakeIdBitVal, fakeCmpIdBitVal, 0);
  innerVAR_OW = processOWData(innerVAR_OW);

  /*checking results*/
  int romBitVal = romNo[0] &0x01; //the 0th bit
  TEST_ASSERT_EQUAL(1, innerVAR_OW.lastZero);
  TEST_ASSERT_EQUAL(2, innerVAR_OW.idBitNumber);
  TEST_ASSERT_EQUAL(0, innerVAR_OW.searchDirection);
  TEST_ASSERT_EQUAL(0, romBitVal);

}

/********************************************************
 * Id_bit     = 0                                       *
 * cmpIdBit = 1                                       *
 *                                                      *
 * EXPECTED:                                            *
 * lastZero = 0                                        *
 * idBitNumber ++                                     *
 * searchDirection = idBit                            *
 * first bit of first byte of romNo = searchDirection *
 ********************************************************/
void test_processOWData_IdBit_cmpBit_01(void){
  /*initialize test*/
  uint8_t fakeIdBitVal []=       {0};
  uint8_t fakeCmpIdBitVal[] =   {1};
  init64BitId(fakeIdBitVal, fakeCmpIdBitVal, 0);
  InnerVAR_OW innerVAR_OW;
  innerVAR_OW = initSearchTest(innerVAR_OW);
  /*initialize condition for test*/
  lastDiscrepancy = 0;
  innerVAR_OW = processOWData(innerVAR_OW);
  int romBitVal = romNo[0] &0x01; //the 0th bit
  /*checking results*/
  TEST_ASSERT_EQUAL(0, innerVAR_OW.lastZero);
  TEST_ASSERT_EQUAL(2, innerVAR_OW.idBitNumber);
  TEST_ASSERT_EQUAL(0, innerVAR_OW.searchDirection);
  TEST_ASSERT_EQUAL(0, romBitVal);

}

/********************************************************
 * Id_bit     = 1                                       *
 * cmpIdBit = 0                                       *
 *                                                      *
 * EXPECTED:                                            *
 * lastZero = 0                                        *
 * idBitNumber ++                                     *
 * searchDirection = idBit                            *
 * first bit of first byte of romNo = searchDirection *
 ********************************************************/
void test_processOWData_IdBit_cmpBit_10(void){
  /*initialize test*/
  uint8_t fakeIdBitVal []=       {1};
  uint8_t fakeCmpIdBitVal[] =   {0};
  init64BitId(fakeIdBitVal, fakeCmpIdBitVal, 0);
  InnerVAR_OW innerVAR_OW;
  innerVAR_OW = initSearchTest(innerVAR_OW);
  /*initialize condition for test*/
  lastDiscrepancy = 0;
  innerVAR_OW = processOWData(innerVAR_OW);
  /*checking results*/
  int romBitVal = romNo[0] &0x01;
  TEST_ASSERT_EQUAL(0, innerVAR_OW.lastZero);
  TEST_ASSERT_EQUAL(2, innerVAR_OW.idBitNumber);
  TEST_ASSERT_EQUAL(1, innerVAR_OW.searchDirection);
  TEST_ASSERT_EQUAL(1, romBitVal);
}

/********************************************************
 * cmpIdBit = 0                                       *
 * Id_bit     = 0                                       *
 *                                                      *
 * EXPECTED:                                            *
 * lastZero = remains                                  *
 * idBitNumber = remains                              *
 * searchDirection = remains                           *
 * searchResult = FALSE                                *
 ********************************************************/
void test_processOWData_IdBit_cmpBit_11(void){
    /*initialize test*/
  uint8_t fakeIdBitVal []=       {1};
  uint8_t fakeCmpIdBitVal[] =   {1};
  init64BitId(fakeIdBitVal, fakeCmpIdBitVal, 0);
  InnerVAR_OW innerVAR_OW;
  innerVAR_OW = initSearchTest(innerVAR_OW);
  /*initialize condition for test*/
  lastDiscrepancy = 0;
  innerVAR_OW = processOWData(innerVAR_OW);
    /*checking results*/
  int romBitVal = romNo[0] &0x01;
  TEST_ASSERT_EQUAL(0, innerVAR_OW.lastZero);
  TEST_ASSERT_EQUAL(1, innerVAR_OW.idBitNumber);
  TEST_ASSERT_EQUAL(0, innerVAR_OW.searchDirection);
  TEST_ASSERT_EQUAL(FALSE, innerVAR_OW.searchResult);
  TEST_ASSERT_EQUAL(0, romBitVal);

}



/********************************************************
 * Id_bit     = 0                                       *
 * cmpIdBit = 0                                       *
 * lastDiscrepancy = 1                                  *
 * idBitNumber = 1                                    *
 *                                                      *
 * EXPECT:                                              *
 * lastZero = 0                                        *
 * idBitNumber++                                      *
 * searchDirection = 1                                 *
 * first bit of first byte of romNo = searchDirection *
 ********************************************************/
void test_processOWData_given_00_lastDiscrepency_sameAs_IDBitNumber_expect_searchDir_1(void){
  /*initialize test*/
  uint8_t fakeIdBitVal []=       {0};
  uint8_t fakeCmpIdBitVal[] =   {0};
  init64BitId(fakeIdBitVal, fakeCmpIdBitVal, 0);
  InnerVAR_OW innerVAR_OW;
  innerVAR_OW = initSearchTest(innerVAR_OW);
  /*Initialize condition of test*/
  //idBit = cmpIdBit = 0
  lastDiscrepancy = 1;
  innerVAR_OW.idBitNumber = 1;
  /*checking results*/
  innerVAR_OW = processOWData(innerVAR_OW);
  int romBitVal = romNo[0] &0x01;
  TEST_ASSERT_EQUAL(1, romBitVal);
  TEST_ASSERT_EQUAL(0, innerVAR_OW.lastZero);
  TEST_ASSERT_EQUAL(2, innerVAR_OW.idBitNumber);
  TEST_ASSERT_EQUAL(1, innerVAR_OW.searchDirection);
}

/********************************************************
 * idBit = 1                                           *
 * cmpIdBit = 0                                       *
 * lastDiscrepancy = 3                                  *
 * idBitNumber = 1                                    *
 * ROM[0] |= 0x01 (set bi0 if ROM[0] to 1)              *
 *                                                      *
 * expect:                                              *
 * lastZero = 0                                        *
 * idBitNumber++                                      *
 * searchDirection = 1                                 *
 * first bit of first byte of romNo = searchDirection *
 ********************************************************/
void test_processOWData_given_00_lastDiscrepency_biggerThan_IDBitNumber_expect_followBack_value_eq_1(void){
  /*initialize test*/
  uint8_t fakeIdBitVal []=       {1};
  uint8_t fakeCmpIdBitVal[] =   {0};
  init64BitId(fakeIdBitVal, fakeCmpIdBitVal, 0);
  InnerVAR_OW innerVAR_OW;
  innerVAR_OW = initSearchTest(innerVAR_OW);
  /*Initialize condition of test*/
  //idBit = cmpIdBit = 0
  lastDiscrepancy = 3;
  innerVAR_OW.idBitNumber = 1;
  romNo[0] |= 0x01;  //set bit 0 to '1'
  /*checking results*/
  innerVAR_OW = processOWData(innerVAR_OW);
  int romBitVal = romNo[0] &0x01;
  TEST_ASSERT_EQUAL(1, romBitVal);
  TEST_ASSERT_EQUAL(0, innerVAR_OW.lastZero);
  TEST_ASSERT_EQUAL(2, innerVAR_OW.idBitNumber);
  TEST_ASSERT_EQUAL(1, innerVAR_OW.searchDirection);

}

/********************************************************
 * idBit = 1                                           *
 * cmpIdBit = 0                                       *
 * lastDiscrepancy = 3                                  *
 * idBitNumber = 1                                    *
 * ROM[0] |= 0x01 (set bi0 if ROM[0] to 0)              *
 *                                                      *
 * expect:                                              *
 * lastZero = 0                                        *
 * idBitNumber++                                      *
 * searchDirection = 0                                 *
 * first bit of first byte of romNo = searchDirection *
 ********************************************************/
void test_processOWData_given_00_lastDiscrepency_biggerThan_IDBitNumber_expect_followBack_romNo_value_eq_0(void){
  /*initialize test*/
  uint8_t fakeIdBitVal []=       {0};
  uint8_t fakeCmpIdBitVal[] =   {0};
  init64BitId(fakeIdBitVal, fakeCmpIdBitVal, 0);
  InnerVAR_OW innerVAR_OW;
  innerVAR_OW = initSearchTest(innerVAR_OW);
  /*Initialize condition of test*/
  //idBit = cmpIdBit = 0
  lastDiscrepancy = 3;
  innerVAR_OW.idBitNumber = 1;
  romNo[0] &= 0xfe;  //set bit 0 to '0'
  /*checking results*/
  innerVAR_OW = processOWData(innerVAR_OW);
  int romBitVal = romNo[0] &0x01;
  TEST_ASSERT_EQUAL(0, romBitVal);
  TEST_ASSERT_EQUAL(1, innerVAR_OW.lastZero);
  TEST_ASSERT_EQUAL(2, innerVAR_OW.idBitNumber);
  TEST_ASSERT_EQUAL(0, innerVAR_OW.searchDirection);

}

/**
 * idBit = 1
 * cmpIdBit = 1
 *
 * expected:
 * _firstSearch(1) return FALSE
 */
void test_search_bit_given_idBit_cmp_idBit_11_expect_SearchFail(void)
{
  /*reset bit and byte pos in return value of OW  */
  uint8_t fakeIdBitVal []=       {1, 0, 1, 0, 1, 1, 0};
  uint8_t fakeCmpIdBitVal[] =   {1, 0, 0, 1, 0, 0, 1};
  init64BitId(fakeIdBitVal, fakeCmpIdBitVal, 0);
  TEST_ASSERT_EQUAL(FALSE, _firstSearch(1));
}

/*Given these data
 *
 * 1 0 0 0  (0x08)    -first data.
 * 0 1 0 0            -second data
 * 0 0 1 0            -third data
 * 0 0 0 1            -forth data

 * lastDiscrepancy = 0

 *(after running through search)...

 *0000...1 0 0 0   <---- This path is chosen, data read is 0x08
 *0000...0 1 0 0
 *0000...0 0 1 0
 *0000...0 0 0 1
           ^
     lastDiscrepancy
points to here so that next search will take the path with "1" (second path)
 *should read these data <true>:<compliment>
 *00 00 00 10 |01 01 01 01....

 *----------------------------------------------------
 *FIRST
 *-----
 *Data retrieved: 0x08 (1.)
 *lastDiscrepancy = 3
 *lastDeviceFlag = FALSE
*/
void test_search_bit_expect_firstdata_LastDisprecancy_3(void)
{
  /*reset bit and byte pos in return value of OW  */
  uint8_t fakeIdBitVal []=       {0, 0, 0, 1, 0, 0, 0, 0};
  uint8_t fakeCmpIdBitVal[] =   {0, 0, 0, 0, 1, 1, 1, 1};
  init64BitId(fakeIdBitVal, fakeCmpIdBitVal, 0);
  TEST_ASSERT_EQUAL(TRUE, _bitSearch(1));
  TEST_ASSERT_EQUAL_INT64(0x08, romNo[0]);
  TEST_ASSERT_EQUAL(3, lastDiscrepancy);
  TEST_ASSERT_EQUAL(FALSE, lastDeviceFlag);
}




/*Given these data
 *
 *1 0 0 0  (0x08)    -first data.   (READ)
 *0 1 0 0            -second data
 *0 0 1 0            -third data
 *0 0 0 1            -forth data
 *  ^
*lastDiscrepancy
 *
*(after running through search).....

 0000...1 0 0 0
 0000...0 1 0 0    <---- This path is chosen, data read is 0x04
 0000...0 0 1 0
 0000...0 0 0 1
            ^
     lastDiscrepancy
points to here so that next search will take the path with "1" (second path)
 *
 *should read these data <true>:<compliment>
 *00 00 00 01 |01 01 01 01....
 */

/*
 *----------------------------------------------------
 *NEXT
 *-----
 *Data to be retrieved: 0x04 (second data)
 *lastDiscrepancy = 2
 *lastDeviceFlag = FALSE
*/
void test_search_bit_expect_SecondData_LastDisprecancy_2(void)
{
  /*Test Initialize*/
  uint8_t fakeIdBitVal []=       {0, 0, 0, 0, 0, 0, 0, 0};
  uint8_t fakeCmpIdBitVal[] =   {0, 0, 0, 1, 1, 1, 1, 1};
  init64BitId(fakeIdBitVal, fakeCmpIdBitVal, 0);

  lastDiscrepancy = 3;
  lastDeviceFlag = FALSE;
  romNo[0] = 0x08;
  TEST_ASSERT_EQUAL(TRUE, _bitSearch(1));
  TEST_ASSERT_EQUAL_INT64(0x04, romNo[0]);
  TEST_ASSERT_EQUAL(2, lastDiscrepancy);
  TEST_ASSERT_EQUAL(FALSE, lastDeviceFlag);
}


/*Given these data
 *
 *1 0 0 0  (0x08)    -first data.(READ)
 *0 1 0 0            -second data(READ)
 *0 0 1 0            -third data
 *0 0 0 1            -forth data
      ^
 * lastDiscrepancy
 *
 *(after running through search)  ...

 *0000...1 0 0 0
 *0000...0 1 0 0
 *0000...0 0 1 0    <---- This path is chosen, data read is 0x04
 *0000...0 0 0 1
               ^
       lastDiscrepancy
 *points to here so that next search will take the path with "1" (second path)
 *should read these data <true>:<compliment>
 *00 00 01 01 |01 01 01 01....
 *----------------------------------------------------
 *NEXT
 *-----
 *Data to be retrieved: 0x02 (third data)
 *lastDiscrepancy = 2
 *lastDeviceFlag = FALSE
*/
void test_search_bit_expect_ThirdData_LastDisprecancy_1(void)
{
  /*Test Initialize*/
  uint8_t fakeIdBitVal []=       {0, 0, 0, 0, 0, 0, 0, 0};
  uint8_t fakeCmpIdBitVal[] =   {0, 0, 1, 1, 1, 1, 1, 1};
  init64BitId(fakeIdBitVal, fakeCmpIdBitVal, 0);

  lastDiscrepancy = 2;
  lastDeviceFlag = FALSE;
  romNo[0] = 0x04;
  TEST_ASSERT_EQUAL(TRUE, _bitSearch(1));
  TEST_ASSERT_EQUAL_INT64(0x02, romNo[0]);
  TEST_ASSERT_EQUAL(1, lastDiscrepancy);
  TEST_ASSERT_EQUAL(FALSE, lastDeviceFlag);
}


/*Given these data
 *
 *|0*61 (assume rest of 61 bit is 0) 1 0 0 0  (0x08)    -first data.(READ)
 *                                   0 1 0 0            -second data.(READ)
 *                                   0 0 1 0            -third data (READ)
 *                                   0 0 0 1            -forth data
                                           ^
 *                                  lastDiscrepancy

 *(after running through search)  ...
 *
 *0000...1 0 0 0
 *0000...0 1 0 0
 *0000...0 0 1 0
 *0000...0 0 0 1    <---- This path is chosen, data read is 0x04

       lastDiscrepancy  = 0
points to zero so that it will return TRUE to lastDeviceFlag
 *should read these data <true>:<compliment>
 *00 01 01 01 |01 01 01 01.... *Note:Lsb comes first
 *----------------------------------------------------
 *NEXT (LAST)
 *-----
 *Data to be retrieved: 0x02 (third data)
 *Expected:
 *lastDiscrepancy = 2
 *lastDeviceFlag = FALSE
*/
void test_search_bit_expect_ForthData_LastDisprecancy_0(void)
{
  /*Test Initialize*/
  uint8_t fakeIdBitVal []=       {0, 0, 0, 0, 0, 0, 0, 0};
  uint8_t fakeCmpIdBitVal[] =   {0, 1, 1, 1, 1, 1, 1, 1};
  init64BitId(fakeIdBitVal, fakeCmpIdBitVal, 0);

  lastDiscrepancy = 1;
  lastDeviceFlag = FALSE;
  romNo[0] = 0x02;
  TEST_ASSERT_EQUAL(TRUE, _bitSearch(1));
  TEST_ASSERT_EQUAL_INT64(0x01, romNo[0]);
  TEST_ASSERT_EQUAL(0, lastDiscrepancy);
  TEST_ASSERT_EQUAL(TRUE, lastDeviceFlag);
}

/* More complex data:
 * 000...0 1 1 0 1 0 1  --> firstData
 * 000...1 0 1 1 0 0 1  --> SecondData
 * 000...0 1 0 0 1 1 0  --> ThirdData
 *
 *expected OW read return: <true><Compliment>
 *00 10 10 01   01 10 01 (thirdData)
 *value to be put into array:
 */

 void test_search_OW_expect_first_dataThree(void){
   /*reset the variables*/
   uint8_t fakeIdBitVal []=       {0, 1, 1, 0, 0, 1, 0, 0};
   uint8_t fakeCmpIdBitVal[] =   {0, 0, 0, 1, 1, 0, 1, 1};
   init64BitId(fakeIdBitVal, fakeCmpIdBitVal, 0);

   TEST_ASSERT_EQUAL(TRUE, _firstSearch(1));
   TEST_ASSERT_EQUAL_INT8(0x26, romNo[0]); //0010 0110
   TEST_ASSERT_EQUAL(1, lastDiscrepancy);
   TEST_ASSERT_EQUAL(FALSE, lastDeviceFlag);
 }

 /* More complex data:
  * 000...0 1 1 0 1 0 1  --> dataOne
  * 000...1 0 1 1 0 0 1  --> dataTwo
  * 000...0 1 0 0 1 1 0  --> dataThree
  *
  *expected OW read return: <true><Compliment>
  *00 01 00 10   10 01 10 (thirdData)
  *value to be put into array:
  */

  void test_search_OW_expect_second_dataTwo(void){
    /*reset the variables*/
    uint8_t fakeIdBitVal []=       {0, 0, 0, 1,  1, 0, 1, 0};
    uint8_t fakeCmpIdBitVal[] =   {0, 1, 0, 0,  0, 1, 0, 1};
    init64BitId(fakeIdBitVal, fakeCmpIdBitVal, 0);

    lastDeviceFlag = FALSE;
    lastDiscrepancy=1;
    romNo[0] = 0x26;
    TEST_ASSERT_EQUAL(TRUE, _bitSearch(1));
    TEST_ASSERT_EQUAL_INT8(0x59, romNo[0]);
    TEST_ASSERT_EQUAL(3, lastDiscrepancy);
    TEST_ASSERT_EQUAL(FALSE, lastDeviceFlag);
  }

  /* More complex data:
   * 000...0 1 1 0 1 0 1  --> dataOne
   * 000...1 0 1 1 0 0 1  --> dataTwo
   * 000...0 1 0 0 1 1 0  --> dataThree
   *
   *expected OW read return: <true><Compliment>
   *00 01 00 01  10 10 01 (thirdData)
   */

   void test_search_OW_expect_third_dataOne(void){
     /*reset the variables*/
     uint8_t fakeIdBitVal []=       {0, 0, 0, 0,  1, 1, 0, 0};
     uint8_t fakeCmpIdBitVal[] =   {0, 1, 0, 1,  0, 0, 1, 1};
     init64BitId(fakeIdBitVal, fakeCmpIdBitVal, 0);

     lastDeviceFlag = FALSE;
     lastDiscrepancy=3;
     romNo[0] = 0x59;
     TEST_ASSERT_EQUAL(TRUE, _bitSearch(1));
     TEST_ASSERT_EQUAL_INT8(0x35, romNo[0]);
     TEST_ASSERT_EQUAL(0, lastDiscrepancy);
     TEST_ASSERT_EQUAL(TRUE, lastDeviceFlag);
   }

   /*Target Setup search (1/3)
   *Given these data
   * 000...0 1 0 1  1 1 0 0 0 1 0 1 --> dataOne   <=====choosen first
   * 000...1 0 1 1  1 1 0 0 0 1 0 1--> dataTwo
   * 000...1 0 0 1  0 0 1 0 0 1 1 0 --> dataThree
   *
   *  * MOCK data:
   *  idBit    :001000111010
   *  cmpIdBit:010111000001
   *  path taken:101000111010
   */

   void test_targetSetupSearch_givenAboveData_expect_dataOne(void){
     uint8_t fakeIdBitVal []=       {0, 0, 1, 0,  0, 0, 1, 1,  1, 0, 1, 0,  0, 0, 0, 0};
     uint8_t fakeCmpIdBitVal[] =   {0, 1, 0, 1,  1, 1, 0, 0,  0, 0, 0, 1,  1, 1, 1, 1};
     init64BitId(fakeIdBitVal, fakeCmpIdBitVal, 0);

     targetSetupSearch(0xc5);
     TEST_ASSERT_EQUAL(TRUE, _bitSearch(2));
     TEST_ASSERT_EQUAL(10, lastDiscrepancy);
     TEST_ASSERT_EQUAL(FALSE, lastDeviceFlag);
     TEST_ASSERT_EQUAL(0xc5, romNo[0]);
     TEST_ASSERT_EQUAL(0x05, romNo[1]);
   }

   /*Target Setup search (2/3)
   *Given these data
   * 000...0 1 0 1  1 1 0 0 0 1 0 1 --> dataOne
   * 000...1 0 1 1  1 1 0 0 0 1 0 1--> dataTwo  <=====choosen second
   * 000...1 0 0 1  0 0 1 0 0 1 1 0 --> dataThree
   *
   * MOCK data:
   *  idBit    :001000111001
   *  cmpIdBit:010111000010
   *  path taken:101000111101
   *
   * * due to we load specific family code, the algorithm couldn't find the device
   * that has different family code. So there is no part 3 (3/3)
   * Expected:  lastDiscrepancy = 0
   *            lastDeviceFlag = TRUE
   *            romNo[1] = 0xb
   */

   void test_targetSetupSearch_cont_givenAboveData_expect_dataTwo(void){
     uint8_t fakeIdBitVal []=       {0, 0, 1, 0,  0, 0, 1, 1,  1, 0, 0, 1,  0, 0, 0, 0};
     uint8_t fakeCmpIdBitVal[] =   {0, 1, 0, 1,  1, 1, 0, 0,  0, 0, 1, 0,  1, 1, 1, 1};
     init64BitId(fakeIdBitVal, fakeCmpIdBitVal, 0);
     //initialize value to continue the targetSetupSearch above
     lastDiscrepancy = 10;
     lastFamilyDiscrepancy = 0;
     lastDeviceFlag = FALSE;
     romNo[0] = 0xc5; //family code
     romNo[1] = 0x05;
     TEST_ASSERT_EQUAL(TRUE, _bitSearch(2));
     TEST_ASSERT_EQUAL(0, lastDiscrepancy);
     TEST_ASSERT_EQUAL(TRUE, lastDeviceFlag);
     TEST_ASSERT_EQUAL(0xc5, romNo[0]);
     TEST_ASSERT_EQUAL(0xb, romNo[1]);
   }

/**
 * The 'VERIFY' operation verifies if a device with a known ROM number is currently connected to the 1-Wire.
 * 'VERIFY' can be perform by:
 * -Suplying the ROM number
 * -set lastDiscrepancy to 64 (8 in this case)
 * -and doing a target search
 * ------------------------------------------------------------
 * Test on 8 bit
 * -------------
 * Given:
 * romNo:   0011 0100
 *          0x34
 * other devices :
 *          0111  1011
 *          0x7b
 *          1011  1000
 *          0xb9
 * --------------
 * MOCK data: idBit    :00001100
 *            cmp-id-bit:01010011
 *
 */
   void test_verify_device_given_device_same_with_ROMNO_EXPECT_same_ROMNO(void){
     uint8_t fakeIdBitVal []=       {0, 0, 0, 0, 1, 1, 0, 0};
     uint8_t fakeCmpIdBitVal[] =   {0, 1, 0, 1, 0, 0, 1, 1};
     init64BitId(fakeIdBitVal, fakeCmpIdBitVal, 0);
     unsigned char romNumberToVerify = 0x34;
     verify(&romNumberToVerify, 1);
     TEST_ASSERT_EQUAL(TRUE, _bitSearch(1));
     TEST_ASSERT_EQUAL(0x34, romNo[0]);

   }
/**
 * Verify certain romNo but given different romNo
 * ------------
 * Test on 8 bitPos
 * ------------
 * Given:
 * romNo to search: 1011 1110
 *                   0xbe
 *Given devices:    0110 1101
 *                  0x6d
 *                  1000 1110
 *                  0x8e
 *--------------------------------------------
 *MOCK data: idBit     :01110001
 *           cmpIdBit :00001110
 */
   void test_verify_device_given_device_different_from_ROMNO_expect_ROMNO_different(void){
     uint8_t fakeIdBitVal []=       {0, 1, 1, 1, 0, 0, 0, 1};
     uint8_t fakeCmpIdBitVal[] =   {0, 0, 0, 0, 1, 1, 1, 0};
     init64BitId(fakeIdBitVal, fakeCmpIdBitVal, 0);
     lastDiscrepancy = 8;
     lastFamilyDiscrepancy = 0;
     lastDeviceFlag = FALSE;
     romNo[0] = 0xbe;
     TEST_ASSERT_EQUAL(TRUE, _bitSearch(1));
     TEST_ASSERT_NOT_EQUAL(0xbe, romNo[0]);
     TEST_ASSERT_EQUAL(0x8e, romNo[0]);
   }
/**
 * The 'FAMILY SKIP SETUP' operation sets the search state to skip all of the
 * devices that have the family code that was found in the previous search.
 *
 * given data:
 *            0011 1001 0110 1010       -> same family code   -->data no.1
 *            1110 0101 0110 1010       ---|                  -->data no.2
 *            0111 1111 1101 0001                             -->data no.3
 *
 * Flow of process:
 * ->first we perform 1wire first search, and it will found data no.1
 * ->perform familySkipSetupSearch and it will found data no.3 instead of data no.2
 *
 *                          ...(after the first search)
 *            0011 1001 0110 1010       -> same family code   -->data no.1
 *            1110 0101 0110 1010       ---|                  -->data no.2
 *            0111 1111 1101 0001                             -->data no.3 -------> this data will chosen in next search
 *                              ^
 *                              |
 *                     lastFamilyDiscrepancy
 * 1wire firstSearch -------------> (1st)
 * ------------------
 * MOCK data: idBit     :0101 0110 1001 1000
 *           cmpIdBit  :0010 1001 0100 0011
 *           path taken  :0101 0110 1001 1000
 *
 * 1wire familySkipSetupSearch -------------> (2nd)
 *MOCK data:  idBit    :0000 1011 1111 1110
 *            cmpIdBit:0111 0100 0000 0001
 *            path taken:1000 1011 1111 1110
 *                       ^
 *                       |
 *         (path taken is 1 because lastDiscrepancy = 1)
 */
   void test_FamilySkipSetup_Search(void){
     /*first search*/
     //----------------------------------------------------
     uint8_t fakeIdBitVal []=       {0, 1, 0, 1,  0, 1, 1, 0,  1, 0, 0, 1,  1, 0, 0, 0};
     uint8_t fakeCmpIdBitVal[] =   {0, 0, 1, 0,  1, 0, 0, 1,  0, 1, 0, 0,  0, 0, 1, 1};
     init64BitId(fakeIdBitVal, fakeCmpIdBitVal, 0);
     /*TEST_ASSERT_EQUAL(TRUE, _firstSearch(2));
     TEST_ASSERT_EQUAL(1, lastFamilyDiscrepancy);
     TEST_ASSERT_EQUAL(11, lastDiscrepancy);*/

     TEST_ASSERT_EQUAL(TRUE, _firstSearch(2));
     TEST_ASSERT_EQUAL(FALSE, lastDeviceFlag);
     TEST_ASSERT_EQUAL(0x6A, romNo[0]);
     /*second search*/
     //----------------------------------------------------

     uint8_t fakeIdBitVal_2 []=       {0, 0, 0, 0,  1, 0, 1, 1,  1, 1, 1, 1,  1, 1, 1, 0};
     uint8_t fakeCmpIdBitVal_2[] =   {0, 1, 1, 1,  0, 1, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1};
     init64BitId(fakeIdBitVal_2, fakeCmpIdBitVal_2, 0);
     familySkipSetupSearch();
     TEST_ASSERT_EQUAL(TRUE ,_bitSearch(2));
     TEST_ASSERT_EQUAL(0x7f, romNo[1]);
     TEST_ASSERT_EQUAL(0xd1, romNo[0]);



   }

   //TODO the rest of targetSetup search
   //TODO test on familyskipSetup and verify
