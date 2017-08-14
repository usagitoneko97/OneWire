#include "unity.h"
#include "search.h"
#include "mock_onewireio.h"
#include "owvariable.h"
#include "common.h"
#include "owcompletesearch.h"
#include <stdlib.h>
// #define NO_OF_DEVICE  4



unsigned char bitPos = 0x01;
int state = 0;
uint8_t *fakeIdBits = NULL;
uint8_t *fakeCmpIdBits = NULL;

#define getBytePos(x)    ((x) >> 3)
#define getBitPos(x)     ((x) & 0x7)

SearchBitType getOwBitState(int devices[][OW_LENGTH], int bitNumber, int numberOfDevices){
  int mBitNumber = (OW_LENGTH - 1) - bitNumber;
  int i;
  int tempResult = devices[0][mBitNumber];
  printf("tempResult%d\n", tempResult);
  for(i = 1; i< numberOfDevices; i++){
    printf("devices :%d\n",devices[i][mBitNumber]);
    if(tempResult != devices[i][mBitNumber]){
      return BIT_CONFLICT;
    }
  }
  if(tempResult == 1){
    return BIT_1;
  }
  else{
    return BIT_0;
  }
}

SearchBitType getSearchBitTypeFrom01(uint8_t fakeIdBits, uint8_t fakeCmpIdBits){
  uint8_t *uartRxVal = (uint8_t*)malloc(2);
  if(fakeIdBits == 1){
    *(uartRxVal) = 0xff;
  } else{
    *(uartRxVal) = 0xef;
  }

  if(fakeCmpIdBits == 1){
    *(uartRxVal + 1) = 0xff;
  } else{
    *(uartRxVal + 1) = 0xef;
  }
  return intepretSearchBit(uartRxVal);
}

void thrashGet1BitRom(BitSearchInformation *bsi, uint8_t *fakeIdBits, uint8_t *fakeCmpIdBits, int numberOfDevices){
  int i = 0;
  while(i++ < OW_LENGTH){
    bsi->bitReadType = getSearchBitTypeFrom01(fakeIdBits[(bsi->idBitNumber-1)], fakeCmpIdBits[(bsi->idBitNumber-1)]);
    get1BitRom(bsi);
    //clearGet1BitRom(bsi);
  }

}

void test_getSearchBitTypeFrom01(void){
  SearchBitType result = getSearchBitTypeFrom01(1, 0);
  TEST_ASSERT_EQUAL(BIT_1, result);

  result = getSearchBitTypeFrom01(0, 1);
  TEST_ASSERT_EQUAL(BIT_0, result);

  result = getSearchBitTypeFrom01(0, 0);
  TEST_ASSERT_EQUAL(BIT_CONFLICT, result);

  result = getSearchBitTypeFrom01(1, 1);
  TEST_ASSERT_EQUAL(DEVICE_NOT_THERE, result);
}


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

void initSearchTest(BitSearchInformation *innerVAR_OW){
  lastDiscrepancy = 0;
  lastDeviceFlag=FALSE;
  lastFamilyDiscrepancy = 0;
  int i = 0;
  while(i<8){
    romNo[i++] = 0;
  }
  innerVAR_OW->idBitNumber = 1;
  innerVAR_OW->lastZero = 0;
  innerVAR_OW->romByteNum = 0;
  innerVAR_OW->byteMask = 1;
  innerVAR_OW->searchResult = 0;
  innerVAR_OW->idBit = -1;
  innerVAR_OW->cmpIdBit = -1;
  innerVAR_OW->searchDirection = 0;
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
 * GIVEN: BIT_CONFLICT                                  *
 *                                                      *
 * EXPECTED:                                            *
 * lastZero = 1                                         *
 * idBitNumber ++                                       *
 * first bit of first byte of romNo = searchDirection   *
 ********************************************************/
void test_process1BitRom_BIT_CONFLICT_idBit_1(void){
  /*initialize test*/
  owLength = 3;
  BitSearchInformation bsi;
  initGet1BitRom(&bsi);
  bsi.bitReadType = BIT_CONFLICT;
  clearGet1BitRom(&bsi);
  bsi.romNo = (uint8_t*)malloc(1);
  *(bsi.romNo) = 0x01;
  get1BitRom(&bsi);

  TEST_ASSERT_EQUAL(1, bsi.lastZero);
  TEST_ASSERT_EQUAL(2, bsi.idBitNumber);
  TEST_ASSERT_EQUAL(0, *(bsi.romNo));
  free(bsi.romNo);
}

/********************************************************
 * Given: BIT_0
 *                                                      *
 * EXPECTED:                                            *
 * lastZero = 0                                        *
 * idBitNumber ++                                     *
 * searchDirection = idBit                            *
 * first bit of first byte of romNo = searchDirection *
 ********************************************************/
void test_process1BitRom_IdBit_cmpBit_01(void){
  /*initialize test*/
  owLength = 3;
  BitSearchInformation bsi;
  initGet1BitRom(&bsi);
  bsi.bitReadType = BIT_0;
  clearGet1BitRom(&bsi);
  bsi.romNo = (uint8_t*)malloc(1);
  *(bsi.romNo) = 0x01;
  get1BitRom(&bsi);

  TEST_ASSERT_EQUAL(0, bsi.lastZero);
  TEST_ASSERT_EQUAL(2, bsi.idBitNumber);
  TEST_ASSERT_EQUAL(0, *(bsi.romNo));
  free(bsi.romNo);

}

/********************************************************
 * Given: BIT_1                                         *
 *                                                      *
 * EXPECTED:                                            *
 * lastZero = 0                                        *
 * idBitNumber ++                                     *
 * searchDirection = idBit                            *
 * first bit of first byte of romNo = searchDirection *
 ********************************************************/
void test_process1BitRom_BIT_1(void){
  /*initialize test*/
  owLength = 3;
  BitSearchInformation bsi;
  initGet1BitRom(&bsi);
  bsi.bitReadType = BIT_1;
  clearGet1BitRom(&bsi);
  bsi.romNo = (uint8_t*)malloc(1);
  *(bsi.romNo) = 0x00;
  get1BitRom(&bsi);

  TEST_ASSERT_EQUAL(0, bsi.lastZero);
  TEST_ASSERT_EQUAL(2, bsi.idBitNumber);
  TEST_ASSERT_EQUAL(1, *(bsi.romNo));
  free(bsi.romNo);

}

/********************************************************
 * Given : DEVICE_NOT_THERE                             *
 *                                                      *
 * EXPECTED:                                            *
 * lastZero = remains                                  *
 * idBitNumber = remains                              *
 * searchDirection = remains                           *
 * searchResult = FALSE                                *
 ********************************************************/
void test_process1BitRom_Given_DEVICE_NOT_THERE(void){
    /*initialize test*/
  owLength = 3;
  BitSearchInformation bsi;
  initGet1BitRom(&bsi);
  bsi.bitReadType = DEVICE_NOT_THERE;
  clearGet1BitRom(&bsi);
  bsi.romNo = (uint8_t*)malloc(1);
  *(bsi.romNo) = 0x00;
  get1BitRom(&bsi);

  TEST_ASSERT_EQUAL(0, bsi.lastZero);
  TEST_ASSERT_EQUAL(1, bsi.idBitNumber);
  TEST_ASSERT_EQUAL(0, *(bsi.romNo));
  TEST_ASSERT_EQUAL(TRUE, bsi.noDevice);
  free(bsi.romNo);
}



/********************************************************
 * Given :BIT_CONFLICT
 * lastDiscrepancy = 1                                  *
 * idBitNumber = 1                                    *
 *                                                      *
 * EXPECT:                                              *
 * lastZero = 0                                        *
 * idBitNumber++                                      *
 * searchDirection = 1                                 *
 * first bit of first byte of romNo = searchDirection *
 ********************************************************/
void test_process1BitRom_given_BIT_CONFLICT_lastDiscrepency_sameAs_IDBitNumber_expect_searchDir_1(void){
  /*initialize test*/
  owLength = 3;
  BitSearchInformation bsi;
  initGet1BitRom(&bsi);
  bsi.bitReadType = BIT_CONFLICT;
  clearGet1BitRom(&bsi);
  bsi.romNo = (uint8_t*)malloc(1);
  *(bsi.romNo) = 0x00;

  lastDiscrepancy = 1;
  get1BitRom(&bsi);

  TEST_ASSERT_EQUAL(0, bsi.lastZero);
  TEST_ASSERT_EQUAL(2, bsi.idBitNumber);
  TEST_ASSERT_EQUAL(1, *(bsi.romNo));
  free(bsi.romNo);
}

/********************************************************
 * Given :BIT_CONFLICT
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
void test_process1BitRom_given_BIT_CONFLICT_lastDiscrepency_biggerThan_IDBitNumber_expect_followBack_value_eq_1(void){
  /*initialize test*/

  owLength = 3;
  BitSearchInformation bsi;
  initGet1BitRom(&bsi);
  bsi.bitReadType = BIT_CONFLICT;
  clearGet1BitRom(&bsi);
  bsi.romNo = (uint8_t*)malloc(1);
  *(bsi.romNo) |= 0x01;
  bsi.idBitNumber = 1;
  lastDiscrepancy = 3;
  get1BitRom(&bsi);

  TEST_ASSERT_EQUAL(0, bsi.lastZero);
  TEST_ASSERT_EQUAL(2, bsi.idBitNumber);
  TEST_ASSERT_EQUAL(1, *(bsi.romNo) & 0x01);
  free(bsi.romNo);
}

/********************************************************
 * Given : BIT_CONFLICT
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
void test_process1BitRom_given_00_lastDiscrepency_biggerThan_IDBitNumber_expect_followBack_romNo_value_eq_0(void){
  /*initialize test*/
  owLength = 3;
  BitSearchInformation bsi;
  initGet1BitRom(&bsi);
  bsi.bitReadType = BIT_CONFLICT;
  clearGet1BitRom(&bsi);
  bsi.romNo = (uint8_t*)malloc(1);
  *(bsi.romNo) &= 0xfe;
  bsi.idBitNumber = 1;
  lastDiscrepancy = 3;
  get1BitRom(&bsi);

  TEST_ASSERT_EQUAL(1, bsi.lastZero);
  TEST_ASSERT_EQUAL(2, bsi.idBitNumber);
  TEST_ASSERT_EQUAL(0, *(bsi.romNo) & 0x01);
  free(bsi.romNo);


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
  uint8_t fakeIdBitVal [] =     {0, 0, 0, 1, 0, 0, 0, 0};
  uint8_t fakeCmpIdBitVal[] =   {0, 0, 0, 0, 1, 1, 1, 1};

  owLength = 8;
  BitSearchInformation bsi;
  initGet1BitRom(&bsi);
  bsi.bitReadType = BIT_CONFLICT;

  bsi.romNo = (uint8_t*)malloc(OW_LENGTH);

  owLength = 4;
  thrashGet1BitRom(&bsi, fakeIdBitVal, fakeCmpIdBitVal, 4);
  TEST_ASSERT_EQUAL(8, (*(bsi.romNo) & 0xf));
  TEST_ASSERT_EQUAL(3, lastDiscrepancy);
  TEST_ASSERT_EQUAL(FALSE, lastDeviceFlag);
  free(bsi.romNo);
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

  owLength = 8;
  BitSearchInformation bsi;
  initGet1BitRom(&bsi);
  bsi.bitReadType = BIT_CONFLICT;

  bsi.romNo = (uint8_t*)malloc(OW_LENGTH);

  /*test prequisite*/
  lastDiscrepancy = 3;
  lastDeviceFlag = FALSE;
  *(bsi.romNo) = 0x08;

  owLength = 4;
  thrashGet1BitRom(&bsi, fakeIdBitVal, fakeCmpIdBitVal, 4);
  TEST_ASSERT_EQUAL(4, (*(bsi.romNo) & 0xf));
  TEST_ASSERT_EQUAL(2, lastDiscrepancy);
  TEST_ASSERT_EQUAL(FALSE, lastDeviceFlag);
  free(bsi.romNo);
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

  owLength = 8;
  BitSearchInformation bsi;
  initGet1BitRom(&bsi);
  bsi.bitReadType = BIT_CONFLICT;

  bsi.romNo = (uint8_t*)malloc(OW_LENGTH);

  /*test prequisite*/
  lastDiscrepancy = 2;
  lastDeviceFlag = FALSE;
  *(bsi.romNo) = 0x04;

  owLength = 4;
  thrashGet1BitRom(&bsi, fakeIdBitVal, fakeCmpIdBitVal, 4);
  TEST_ASSERT_EQUAL(2, (*(bsi.romNo) & 0xf));
  TEST_ASSERT_EQUAL(1, lastDiscrepancy);
  TEST_ASSERT_EQUAL(FALSE, lastDeviceFlag);
  free(bsi.romNo);
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

  owLength = 8;
  BitSearchInformation bsi;
  initGet1BitRom(&bsi);
  bsi.bitReadType = BIT_CONFLICT;

  bsi.romNo = (uint8_t*)malloc(OW_LENGTH);

  /*test prequisite*/
  lastDiscrepancy = 1;
  lastDeviceFlag = FALSE;
  *(bsi.romNo) = 0x02;

  owLength = 4;
  thrashGet1BitRom(&bsi, fakeIdBitVal, fakeCmpIdBitVal, 4);
  TEST_ASSERT_EQUAL(1, (*(bsi.romNo) & 0xf));
  TEST_ASSERT_EQUAL(0, lastDiscrepancy);
  TEST_ASSERT_EQUAL(TRUE, lastDeviceFlag);
  free(bsi.romNo);
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

   owLength = 8;
   BitSearchInformation bsi;
   initGet1BitRom(&bsi);
   bsi.bitReadType = BIT_CONFLICT;
  //  clearGet1BitRom(&bsi);
   bsi.romNo = (uint8_t*)malloc(OW_LENGTH);

   thrashGet1BitRom(&bsi, fakeIdBitVal, fakeCmpIdBitVal, 4);
   TEST_ASSERT_EQUAL(0x26, (*(bsi.romNo)));
   TEST_ASSERT_EQUAL(1, lastDiscrepancy);
   TEST_ASSERT_EQUAL(FALSE, lastDeviceFlag);
   free(bsi.romNo);
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

    owLength = 8;
    BitSearchInformation bsi;
    // initGet1BitRom(&bsi);
    bsi.bitReadType = BIT_CONFLICT;
    clearGet1BitRom(&bsi);
    bsi.romNo = (uint8_t*)malloc(OW_LENGTH);
    lastDeviceFlag = FALSE;
    lastDiscrepancy=1;
    *(bsi.romNo) = 0x26;

    thrashGet1BitRom(&bsi, fakeIdBitVal, fakeCmpIdBitVal, 4);
    TEST_ASSERT_EQUAL(0x59, (*(bsi.romNo)));
    TEST_ASSERT_EQUAL(3, lastDiscrepancy);
    TEST_ASSERT_EQUAL(FALSE, lastDeviceFlag);
    free(bsi.romNo);
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

     owLength = 8;
     BitSearchInformation bsi;
     // initGet1BitRom(&bsi);
     bsi.bitReadType = BIT_CONFLICT;
     clearGet1BitRom(&bsi);
     bsi.romNo = (uint8_t*)malloc(OW_LENGTH);
     lastDeviceFlag = FALSE;
     lastDiscrepancy=3;
     *(bsi.romNo) = 0x59;

     thrashGet1BitRom(&bsi, fakeIdBitVal, fakeCmpIdBitVal, 4);
     TEST_ASSERT_EQUAL(0x35, (*(bsi.romNo)));
     TEST_ASSERT_EQUAL(0, lastDiscrepancy);
     TEST_ASSERT_EQUAL(TRUE, lastDeviceFlag);
     free(bsi.romNo);
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

     owLength = 16;
     BitSearchInformation bsi;
     // initGet1BitRom(&bsi);
     clearGet1BitRom(&bsi);
     bsi.romNo = (uint8_t*)malloc(OW_LENGTH);

     targetSetupConfig(0xc5, &bsi);
     thrashGet1BitRom(&bsi, fakeIdBitVal, fakeCmpIdBitVal, 3);
     TEST_ASSERT_EQUAL(10, lastDiscrepancy);
     TEST_ASSERT_EQUAL(FALSE, lastDeviceFlag);
     TEST_ASSERT_EQUAL(0xc5, (*(bsi.romNo)));
     TEST_ASSERT_EQUAL(0x05, (*(bsi.romNo + 1)));
     free(bsi.romNo);
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

     owLength = 16;
     BitSearchInformation bsi;
     // initGet1BitRom(&bsi);
     clearGet1BitRom(&bsi);
     bsi.romNo = (uint8_t*)malloc(OW_LENGTH);

     lastDiscrepancy = 10;
     lastFamilyDiscrepancy = 0;
     lastDeviceFlag = FALSE;
     bsi.romNo[0] = 0xc5; //family code
     bsi.romNo[1] = 0x05;

     thrashGet1BitRom(&bsi, fakeIdBitVal, fakeCmpIdBitVal, 3);
     TEST_ASSERT_EQUAL(0, lastDiscrepancy);
     TEST_ASSERT_EQUAL(TRUE, lastDeviceFlag);
     TEST_ASSERT_EQUAL(0xc5, (*(bsi.romNo)));
     TEST_ASSERT_EQUAL(0xb, (*(bsi.romNo + 1)));
     free(bsi.romNo);
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

     owLength = 8;
     BitSearchInformation bsi;
     // initGet1BitRom(&bsi);
     clearGet1BitRom(&bsi);
     bsi.romNo = (uint8_t*)malloc(OW_LENGTH);

     uint8_t romNumberToVerify = 0x34;
     verifyConfig(&romNumberToVerify, 1, &bsi);

     thrashGet1BitRom(&bsi, fakeIdBitVal, fakeCmpIdBitVal, 3);
     TEST_ASSERT_EQUAL(0x34, bsi.romNo[0]);
     free(bsi.romNo);
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

     owLength = 8;
     BitSearchInformation bsi;
     // initGet1BitRom(&bsi);
     clearGet1BitRom(&bsi);
     bsi.romNo = (uint8_t*)malloc(OW_LENGTH);
     lastDiscrepancy = 8;
     lastFamilyDiscrepancy = 0;
     lastDeviceFlag = FALSE;
     bsi.romNo[0] = 0xbe;
     thrashGet1BitRom(&bsi, fakeIdBitVal, fakeCmpIdBitVal, 3);
     TEST_ASSERT_NOT_EQUAL(0xbe, bsi.romNo[0]);
     TEST_ASSERT_EQUAL(0x8e, bsi.romNo[0]);
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

   owLength = 16;
   BitSearchInformation bsi;
   initGet1BitRom(&bsi);
   clearGet1BitRom(&bsi);
   bsi.romNo = (uint8_t*)malloc(OW_LENGTH);
   thrashGet1BitRom(&bsi, fakeIdBitVal, fakeCmpIdBitVal, 3);

   TEST_ASSERT_EQUAL(FALSE, lastDeviceFlag);
   TEST_ASSERT_EQUAL(0x6A, bsi.romNo[0]);
   /*second search*/
   //----------------------------------------------------

   uint8_t fakeIdBitVal_2 []=      {0, 0, 0, 0,  1, 0, 1, 1,  1, 1, 1, 1,  1, 1, 1, 0};
   uint8_t fakeCmpIdBitVal_2[] =   {0, 1, 1, 1,  0, 1, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1};

   clearGet1BitRom(&bsi);
   familySkipConfig();
   thrashGet1BitRom(&bsi, fakeIdBitVal_2, fakeCmpIdBitVal_2, 3);

   TEST_ASSERT_EQUAL(0x7f, bsi.romNo[1]);
   TEST_ASSERT_EQUAL(0xd1, bsi.romNo[0]);
 }

 /**
  * data one: 1 1 0
  * data two: 0 1 0
  */
 void test_get1BitRom_given_given_data_above(void){
   owLength = 3;
   BitSearchInformation bsi;
   initGet1BitRom(&bsi);
   bsi.bitReadType = BIT_0;
   get1BitRom(&bsi);
   TEST_ASSERT_EQUAL(2, bsi.idBitNumber);
   TEST_ASSERT_EQUAL(0, bsi.lastZero);
   TEST_ASSERT_EQUAL(lastDiscrepancy, 0);

   bsi.bitReadType = BIT_1;
   get1BitRom(&bsi);
   TEST_ASSERT_EQUAL(3, bsi.idBitNumber);
   TEST_ASSERT_EQUAL(0, bsi.lastZero);
   TEST_ASSERT_EQUAL(lastDiscrepancy, 0);

   bsi.bitReadType = BIT_CONFLICT;
   get1BitRom(&bsi);
   TEST_ASSERT_EQUAL(1, bsi.idBitNumber);
   TEST_ASSERT_EQUAL(0, bsi.lastZero);
   TEST_ASSERT_EQUAL(3, lastDiscrepancy);
   //===================================================
   bsi.bitReadType = BIT_0;
   get1BitRom(&bsi);
   TEST_ASSERT_EQUAL(2, bsi.idBitNumber);
   TEST_ASSERT_EQUAL(0, bsi.lastZero);
   TEST_ASSERT_EQUAL(3, lastDiscrepancy);

   bsi.bitReadType = BIT_1;
   get1BitRom(&bsi);
   TEST_ASSERT_EQUAL(3, bsi.idBitNumber);
   TEST_ASSERT_EQUAL(0, bsi.lastZero);
   TEST_ASSERT_EQUAL(3, lastDiscrepancy);

   bsi.bitReadType = BIT_CONFLICT;
   get1BitRom(&bsi);
   TEST_ASSERT_EQUAL(1, bsi.idBitNumber);
   TEST_ASSERT_EQUAL(0, bsi.lastZero);
   TEST_ASSERT_EQUAL(0, lastDiscrepancy);

 }

 void test_GET_CURRENT_BIT_IN_ROM(void){
   BitSearchInformation *bsi ;
   bsi = (BitSearchInformation*)malloc(sizeof(BitSearchInformation));
   bsi->romNo = malloc(8);
   bsi->romByteNum = 1;
   bsi->byteMask = 0x04;
   *(bsi->romNo + 1) = 0x55;
   TEST_ASSERT_EQUAL(1, GET_CURRENT_BIT_IN_ROM(bsi));
   free(bsi->romNo);
   free(bsi);
 }

/**
 * Initialize
 * ----------------
 * romNo[1] = 0 1 0 1 0 1 0 1
 * byteMask = 0 0 0 0 0 0 1 0
 * -----------------
 * result
 * romNo[1] = 0 1 0 1 0 1 1 1
 */
 void test_SET_ROM_BIT(void){
   BitSearchInformation *bsi ;
   bsi = (BitSearchInformation*)malloc(sizeof(BitSearchInformation));
   bsi->romNo = malloc(8);
   bsi->romByteNum = 1;
   bsi->byteMask = 0x02;
   *(bsi->romNo + 1) = 0x55;
   TEST_ASSERT_EQUAL(0x57, SET_ROM_BIT(bsi));
   free(bsi->romNo);
   free(bsi);
 }

 /**
  * Initialize
  * ----------------
  * romNo[1] = 0 1 0 1 0 1 0 1
  * byteMask = 0 0 0 0 0 1 0 0
  * -----------------
  * result
  * romNo[1] = 0 1 0 1 0 0 0 1
  */
 void test_RESET_ROM_BIT(void){
   BitSearchInformation *bsi ;
   bsi = (BitSearchInformation*)malloc(sizeof(BitSearchInformation));
   bsi->romNo = malloc(8);
   bsi->romByteNum = 1;
   bsi->byteMask = 0x04;
   *(bsi->romNo + 1) = 0x55;
   TEST_ASSERT_EQUAL(0x51, RESET_ROM_BIT(bsi));
   free(bsi->romNo);
   free(bsi);
 }

 void test_UPDATE_LAST_FAMILY_DISCREPANCY_given_lastZero_lessThan_FAMILYCODE(void){
   BitSearchInformation *bsi ;
   bsi = (BitSearchInformation*)malloc(sizeof(BitSearchInformation));
   bsi->lastZero = 3;
   lastFamilyDiscrepancy = 0;
   UPDATE_LAST_FAMILY_DISCREPANCY(bsi);
   TEST_ASSERT_EQUAL(3, lastFamilyDiscrepancy);
   free(bsi);
 }

 void test_UPDATE_LAST_FAMILY_DISCREPANCY_given_lastZero_greaterThan_FAMILYCODE(void){
   BitSearchInformation *bsi ;
   bsi = (BitSearchInformation*)malloc(sizeof(BitSearchInformation));
   bsi->lastZero = 9;
   lastFamilyDiscrepancy = 0;
   UPDATE_LAST_FAMILY_DISCREPANCY(bsi);
   TEST_ASSERT_EQUAL(0, lastFamilyDiscrepancy);
   free(bsi);
 }

 void test_UPDATE_ROM_BYTE_MASK_given_mask_0x40_byteNum2_expect_byteNum2(void){
   BitSearchInformation *bsi ;
   bsi = (BitSearchInformation*)malloc(sizeof(BitSearchInformation));
   bsi->byteMask = 0x40;
   bsi->romByteNum = 2;
   UPDATE_ROM_BYTE_MASK(bsi);
   TEST_ASSERT_EQUAL(2, bsi->romByteNum);
   TEST_ASSERT_EQUAL(0x80, bsi->byteMask);
   free(bsi);
 }

 void test_UPDATE_ROM_BYTE_MASK_given_mask_0x80_byteNum2_expect_byteNum3(void){
   BitSearchInformation *bsi ;
   bsi = (BitSearchInformation*)malloc(sizeof(BitSearchInformation));
   bsi->byteMask = 0x80;
   bsi->romByteNum = 2;
   UPDATE_ROM_BYTE_MASK(bsi);
   TEST_ASSERT_EQUAL(3, bsi->romByteNum);
   TEST_ASSERT_EQUAL(0x1, bsi->byteMask);
   free(bsi);
 }

/**
 * Given : idBitNumber = 65
 *         owLength = 64
 *         lastZero = 0
 *
 * Expect: lastDeviceFlag = TRUE
 *         lastZero = 0;
 *         searchResult = TRUE
 *         lastDiscrepancy = 0
 *         idBitNumber = 1
 */
 void test_RESET_IF_COMPLETED_BIT_SEARCHING_given_Expect_above(void){
   BitSearchInformation *bsi ;
   bsi = (BitSearchInformation*)malloc(sizeof(BitSearchInformation));
   bsi->lastZero = 0;
   bsi->idBitNumber = 65;
   owLength = 64;
   RESET_IF_COMPLETED_BIT_SEARCHING(bsi);
   TEST_ASSERT_EQUAL(TRUE, lastDeviceFlag);
   TEST_ASSERT_EQUAL(0, bsi->lastZero);
   TEST_ASSERT_EQUAL(TRUE, bsi->searchResult);
   TEST_ASSERT_EQUAL(0, lastDiscrepancy);
   TEST_ASSERT_EQUAL(1, bsi->idBitNumber);
   free(bsi);
 }


 /**
  * Given : idBitNumber = 65
  *         owLength = 64
  *         lastZero = 3
  *
  * Expect: lastDeviceFlag = FALSE
  *         lastZero = 0;
  *         searchResult = TRUE
  *         lastDiscrepancy = 3
  *         idBitNumber = 1
  */
  void test_RESET_IF_COMPLETED_BIT_SEARCHING_given_LastZero_1_Expect_above(void){
    BitSearchInformation *bsi ;
    bsi = (BitSearchInformation*)malloc(sizeof(BitSearchInformation));
    bsi->idBitNumber = 65;
    owLength = 64;
    bsi->lastZero = 3;
    lastDeviceFlag = FALSE;
    RESET_IF_COMPLETED_BIT_SEARCHING(bsi);
    TEST_ASSERT_EQUAL(FALSE, lastDeviceFlag);
    TEST_ASSERT_EQUAL(0, bsi->lastZero);
    TEST_ASSERT_EQUAL(TRUE, bsi->searchResult);
    TEST_ASSERT_EQUAL(3, lastDiscrepancy);
    TEST_ASSERT_EQUAL(1, bsi->idBitNumber);
    free(bsi);
  }

/**
 * given: bsi->idBitNumber = 64
 *                owLength = 63
 * Expect: NO CHANGES
 */
  void test_RESET_IF_COMPLETED_BIT_SEARCHING_given_BitNumber_smallerThan_owLength(void){
    BitSearchInformation *bsi ;
    bsi = (BitSearchInformation*)malloc(sizeof(BitSearchInformation));
    bsi->idBitNumber = 63;
    owLength = 63;
    bsi->lastZero = 3;
    lastDeviceFlag = FALSE;
    bsi->searchResult = FALSE;
    lastDiscrepancy = 0;
    RESET_IF_COMPLETED_BIT_SEARCHING(bsi);
    TEST_ASSERT_EQUAL(FALSE, lastDeviceFlag);
    TEST_ASSERT_EQUAL(3, bsi->lastZero);
    TEST_ASSERT_EQUAL(FALSE, bsi->searchResult);
    TEST_ASSERT_EQUAL(0, lastDiscrepancy);
    free(bsi);
  }

  void test_getOwBitState_given_array_expect_SearchBitType(void){
    owLength = 5;
    int devices[3][5] = {{1, 0, 1, 1, 0},
                         {0, 0, 1, 0, 1},
                         {1, 0, 1, 0, 1}};

    TEST_ASSERT_EQUAL(BIT_CONFLICT, getOwBitState(devices, 1, 3));
    TEST_ASSERT_EQUAL(BIT_0, getOwBitState(devices, 3, 3));
    TEST_ASSERT_EQUAL(BIT_1, getOwBitState(devices, 2, 3));
  }


   //TODO the rest of targetSetup search
   //TODO test on familyskipSetup and verify
