#include "unity.h"
#include "Search.h"
#include "mock_OneWireio.h"
#include "OwVariable.h"
#include "common.h"
#include "OwCompleteSearch.h"
#include <stdlib.h>
#include "Callback.h"
#include "LinkedList.h"

void muteConflictDevice(int devices[][OW_LENGTH], int numberOfDevices, int bitNumber, int searchDir);
void resetDeviceListTo1();

unsigned char bitPos = 0x01;
int state = 0;
uint8_t *fakeIdBits = NULL;
uint8_t *fakeCmpIdBits = NULL;

int deviceList[255] = {[0 ... 254] = 1};  //initialize all to 1
#define getBytePos(x)    ((x) >> 3)
#define getBitPos(x)     ((x) & 0x7)

/**
 * @brief  get the SearchBitType of bit bitNumber of the devices
 * @param  devices         the n x 64bits array of the rom number of the devices
 * @param  bitNumber       the bitNumber of the devices to get the Search
 * @param  numberOfDevices the number of 1 wire devices
 * @test   test_getOwBitState_given_array_expect_SearchBitType
 * @return                 SearchBitType, can either be BIT_1, BIT_0 or BIT_CONFLICT
 */
SearchBitType getOwBitState(int devices[][OW_LENGTH], int bitNumber, int numberOfDevices){
  int mBitNumber = (OW_LENGTH - 1) - bitNumber;
  int firstResultDvNumber;
  int firstResult;
  int i;
  //get the first bit number
  for(firstResultDvNumber = 0; firstResultDvNumber<numberOfDevices; firstResultDvNumber++){
    if(deviceList[firstResultDvNumber] == 1){
      firstResult = devices[firstResultDvNumber][mBitNumber];
      break;
    }
  }
  // if(deviceList[0] == 1)
    // int tempResult = devices[0][mBitNumber];
  for(i = firstResultDvNumber; i< numberOfDevices; i++){
    if(deviceList[i] == 1){
      if(firstResult != devices[i][mBitNumber]){
        return BIT_CONFLICT;
      }
    }
  }
  if(firstResult == 1){
    return BIT_1;
  }
  else{
    return BIT_0;
  }
}

void muteConflictDevice(int devices[][OW_LENGTH], int numberOfDevices, int bitNumber, int searchDir){
  int i = 0;
  int mBitNumber = (OW_LENGTH - 1) - bitNumber;
  for (i = 0; i<numberOfDevices; i++){
    if (devices[i][mBitNumber] != searchDir){
      //mute them
      deviceList[i] = 0;
    }
  }
}

void resetDeviceListTo1(){
  int i;
  for(i = 0; i<255; i++){
    deviceList[i] = 1;
  }
}



/**
 * @NOTE for testing purpose
 * @brief to test out the complete rom number which bit number is specify by owLength
 * @param bsi               pointer of bitSearchInformation (contain all information aboute the bit search)
 * @param devices           2d arrays of rom UID
 * @param numberOfDevices   number of devices in the bus
 */
void get1BitRomLoop(BitSearchInformation *bsi, int devices[][OW_LENGTH], int numberOfDevices){
  int i = 0;
  int mBitNumber;
  while(i < OW_LENGTH){
    bsi->bitReadType = getOwBitState(devices, i, numberOfDevices);
    get1BitRom(bsi);
    muteConflictDevice(devices, numberOfDevices, i, searchDir);
    i++;
  }
}

void fakeWrite(unsigned char byte, int numOfCalls){
}


void setUp(void)
{
  write_StubWithCallback(fakeWrite);
}

void tearDown(void) {
}

void initSearchTest(BitSearchInformation *innerVAR_OW){
  lastDiscrepancy = 0;
  lastDeviceFlag=FALSE;
  lastFamilyDiscrepancy = 0;
  int i = 0;
  while(i<8){
    romUid[i++] = 0;
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

/**
 * given data : 1 0 1 1
 *            : 0 1 1 0
 *            : 1 0 0 1
 */
void test_muteConflictDevice_given_101_searchDir_0_expect_data1_and_data2_muted(void){
  owLength = 4;
  int devices[3][4] = {{1, 0, 1, 1},
                       {0, 1, 1, 0},
                       {1, 0, 0, 1}};

  resetDeviceListTo1();
  muteConflictDevice(devices, 3, 3, 0);
  TEST_ASSERT_EQUAL_INT(0, deviceList[0]);
  TEST_ASSERT_EQUAL_INT(1, deviceList[1]);
  TEST_ASSERT_EQUAL_INT(0, deviceList[2]);
}

void test_getOwBitState_given_array_expect_SearchBitType(void){
  owLength = 5;
  int devices[3][5] = {{1, 0, 1, 1, 0},
                       {0, 0, 1, 0, 1},
                       {1, 0, 1, 0, 1}};

  resetDeviceListTo1();
  deviceList[0] = 0;  //mute the 0th device
  TEST_ASSERT_EQUAL(BIT_1, getOwBitState(devices, 0, 3));
  deviceList[0] = 1;
  deviceList[1] = 0;
  TEST_ASSERT_EQUAL(BIT_1, getOwBitState(devices, 4, 3));
  // TEST_ASSERT_EQUAL(BIT_0, getOwBitState(devices, 3, 3));
  // TEST_ASSERT_EQUAL(BIT_1, getOwBitState(devices, 2, 3));
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
  owSetUpRxIT_Expect(uartRxDataBuffer, 3);
  get1BitRom(&bsi);
  TEST_ASSERT_EQUAL(2, bsi.idBitNumber);
  TEST_ASSERT_EQUAL(0, bsi.lastZero);
  TEST_ASSERT_EQUAL(lastDiscrepancy, 0);

  bsi.bitReadType = BIT_1;
  owSetUpRxIT_Expect(uartRxDataBuffer, 3);
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
  owSetUpRxIT_Expect(uartRxDataBuffer, 3);
  get1BitRom(&bsi);
  TEST_ASSERT_EQUAL(2, bsi.idBitNumber);
  TEST_ASSERT_EQUAL(0, bsi.lastZero);
  TEST_ASSERT_EQUAL(3, lastDiscrepancy);

  bsi.bitReadType = BIT_1;
  owSetUpRxIT_Expect(uartRxDataBuffer, 3);
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

/********************************************************
 * GIVEN: BIT_CONFLICT                                  *
 *                                                      *
 * EXPECTED:                                            *
 * lastZero = 1                                         *
 * idBitNumber ++                                       *
 * first bit of first byte of romUid = searchDirection   *
 ********************************************************/
void test_process1BitRom_BIT_CONFLICT_idBit_1(void){
  /*initialize test*/
  owLength = 3;
  BitSearchInformation bsi;
  initGet1BitRom(&bsi);
  bsi.bitReadType = BIT_CONFLICT;
  clearGet1BitRom(&bsi);
  bsi.romUid = (uint8_t*)malloc(1);
  *(bsi.romUid) = 0x01;
  owSetUpRxIT_Expect(uartRxDataBuffer, 3);
  get1BitRom(&bsi);

  TEST_ASSERT_EQUAL(1, bsi.lastZero);
  TEST_ASSERT_EQUAL(2, bsi.idBitNumber);
  TEST_ASSERT_EQUAL(0, *(bsi.romUid));
  free(bsi.romUid);
}

/********************************************************
 * Given: BIT_0                                         *
 *                                                      *
 * EXPECTED:                                            *
 * lastZero = 0                                         *
 * idBitNumber ++                                       *
 * searchDirection = idBit                              *
 * first bit of first byte of romUid = searchDirection   *
 ********************************************************/
void test_process1BitRom_IdBit_cmpBit_01(void){
  /*initialize test*/
  owLength = 3;
  BitSearchInformation bsi;
  initGet1BitRom(&bsi);
  bsi.bitReadType = BIT_0;
  clearGet1BitRom(&bsi);
  bsi.romUid = (uint8_t*)malloc(1);
  *(bsi.romUid) = 0x01;

  owSetUpRxIT_Expect(uartRxDataBuffer, 3);

  get1BitRom(&bsi);

  TEST_ASSERT_EQUAL(0, bsi.lastZero);
  TEST_ASSERT_EQUAL(2, bsi.idBitNumber);
  TEST_ASSERT_EQUAL(0, *(bsi.romUid));
  free(bsi.romUid);

}

/********************************************************
 * Given: BIT_1                                         *
 *                                                      *
 * EXPECTED:                                            *
 * lastZero = 0                                         *
 * idBitNumber ++                                       *
 * searchDirection = idBit                              *
 * first bit of first byte of romUid = searchDirection   *
 ********************************************************/
void test_process1BitRom_BIT_1(void){
  /*initialize test*/
  owLength = 3;
  BitSearchInformation bsi;
  initGet1BitRom(&bsi);
  bsi.bitReadType = BIT_1;
  clearGet1BitRom(&bsi);
  bsi.romUid = (uint8_t*)malloc(1);
  *(bsi.romUid) = 0x00;

  owSetUpRxIT_Expect(uartRxDataBuffer, 3);

  get1BitRom(&bsi);

  TEST_ASSERT_EQUAL(0, bsi.lastZero);
  TEST_ASSERT_EQUAL(2, bsi.idBitNumber);
  TEST_ASSERT_EQUAL(1, *(bsi.romUid));
  free(bsi.romUid);

}

/********************************************************
 * Given : DEVICE_NOT_THERE                             *
 *                                                      *
 * EXPECTED:                                            *
 * lastZero = remains                                   *
 * idBitNumber = remains                                *
 * searchDirection = remains                            *
 * searchResult = FALSE                                 *
 ********************************************************/
void test_process1BitRom_Given_DEVICE_NOT_THERE(void){
    /*initialize test*/
  owLength = 3;
  BitSearchInformation bsi;
  initGet1BitRom(&bsi);
  bsi.bitReadType = DEVICE_NOT_THERE;
  clearGet1BitRom(&bsi);
  bsi.romUid = (uint8_t*)malloc(1);
  *(bsi.romUid) = 0x00;
  get1BitRom(&bsi);

  TEST_ASSERT_EQUAL(0, bsi.lastZero);
  TEST_ASSERT_EQUAL(1, bsi.idBitNumber);
  TEST_ASSERT_EQUAL(0, *(bsi.romUid));
  TEST_ASSERT_EQUAL(TRUE, bsi.noDevice);
  free(bsi.romUid);
}



/********************************************************
 * Given :BIT_CONFLICT                                  *
 * lastDiscrepancy = 1                                  *
 * idBitNumber = 1                                      *
 *                                                      *
 * EXPECT:                                              *
 * lastZero = 0                                         *
 * idBitNumber++                                        *
 * searchDirection = 1                                  *
 * first bit of first byte of romUid = searchDirection   *
 ********************************************************/
void test_process1BitRom_given_BIT_CONFLICT_lastDiscrepency_sameAs_IDBitNumber_expect_searchDir_1(void){
  /*initialize test*/
  owLength = 3;
  BitSearchInformation bsi;
  initGet1BitRom(&bsi);
  bsi.bitReadType = BIT_CONFLICT;
  clearGet1BitRom(&bsi);
  bsi.romUid = (uint8_t*)malloc(1);
  *(bsi.romUid) = 0x00;

  lastDiscrepancy = 1;

  owSetUpRxIT_Expect(uartRxDataBuffer, 3);

  get1BitRom(&bsi);

  TEST_ASSERT_EQUAL(0, bsi.lastZero);
  TEST_ASSERT_EQUAL(2, bsi.idBitNumber);
  TEST_ASSERT_EQUAL(1, *(bsi.romUid));
  free(bsi.romUid);
}

/********************************************************
 * Given :BIT_CONFLICT                                  *
 * lastDiscrepancy = 3                                  *
 * idBitNumber = 1                                      *
 * ROM[0] |= 0x01 (set bi0 if ROM[0] to 1)              *
 *                                                      *
 * expect:                                              *
 * lastZero = 0                                         *
 * idBitNumber++                                        *
 * searchDirection = 1                                  *
 * first bit of first byte of romUid = searchDirection   *
 ********************************************************/
void test_process1BitRom_given_BIT_CONFLICT_lastDiscrepency_biggerThan_IDBitNumber_expect_followBack_value_eq_1(void){
  /*initialize test*/

  owLength = 3;
  BitSearchInformation bsi;
  initGet1BitRom(&bsi);
  bsi.bitReadType = BIT_CONFLICT;
  clearGet1BitRom(&bsi);
  bsi.romUid = (uint8_t*)malloc(1);
  *(bsi.romUid) |= 0x01;
  bsi.idBitNumber = 1;
  lastDiscrepancy = 3;

  owSetUpRxIT_Expect(uartRxDataBuffer, 3);

  get1BitRom(&bsi);

  TEST_ASSERT_EQUAL(0, bsi.lastZero);
  TEST_ASSERT_EQUAL(2, bsi.idBitNumber);
  TEST_ASSERT_EQUAL(1, *(bsi.romUid) & 0x01);
  free(bsi.romUid);
}

/********************************************************
 * Given : BIT_CONFLICT                                 *
 * lastDiscrepancy = 3                                  *
 * idBitNumber = 1                                      *
 * ROM[0] |= 0x01 (set bi0 if ROM[0] to 0)              *
 *                                                      *
 * expect:                                              *
 * lastZero = 0                                         *
 * idBitNumber++                                        *
 * searchDirection = 0                                  *
 * first bit of first byte of romUid = searchDirection   *
 ********************************************************/
void test_process1BitRom_given_00_lastDiscrepency_biggerThan_IDBitNumber_expect_followBack_romUid_value_eq_0(void){
  /*initialize test*/
  owLength = 3;
  BitSearchInformation bsi;
  initGet1BitRom(&bsi);
  bsi.bitReadType = BIT_CONFLICT;
  clearGet1BitRom(&bsi);
  bsi.romUid = (uint8_t*)malloc(1);
  *(bsi.romUid) &= 0xfe;
  bsi.idBitNumber = 1;
  lastDiscrepancy = 3;

  owSetUpRxIT_Expect(uartRxDataBuffer, 3);

  get1BitRom(&bsi);

  TEST_ASSERT_EQUAL(1, bsi.lastZero);
  TEST_ASSERT_EQUAL(2, bsi.idBitNumber);
  TEST_ASSERT_EQUAL(0, *(bsi.romUid) & 0x01);
  free(bsi.romUid);


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
  owLength = 8;
  int devices[4][4] = {{1, 0, 0, 0},
                       {0, 1, 0, 0},
                       {0, 0, 1, 0},
                       {0, 0, 0, 1}};

  //reset the device list to 1 (unmute all device)
  resetDeviceListTo1();
  BitSearchInformation bsi;
  initGet1BitRom(&bsi);
  bsi.romUid = (uint8_t*)malloc(OW_LENGTH);
  owLength = 4;
  int count = 0;
  //set up received interrupt, but didnt setup when reading final bits
  for(count = 0; count <(owLength-1) ;count++){
    owSetUpRxIT_Expect(uartRxDataBuffer, 3);
  }
  get1BitRomLoop(&bsi, devices, 4);
  TEST_ASSERT_EQUAL(8, (*(bsi.romUid) & 0xf));
  TEST_ASSERT_EQUAL(3, lastDiscrepancy);
  TEST_ASSERT_EQUAL(FALSE, lastDeviceFlag);
  free(bsi.romUid);
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
  int devices[4][4] = {{1, 0, 0, 0},
                       {0, 1, 0, 0},
                       {0, 0, 1, 0},
                       {0, 0, 0, 1}};
  resetDeviceListTo1();
  BitSearchInformation bsi;
  initGet1BitRom(&bsi);
  bsi.romUid = (uint8_t*)malloc(OW_LENGTH);
  /*test prequisite*/
  lastDiscrepancy = 3;
  lastDeviceFlag = FALSE;
  *(bsi.romUid) = 0x08;
  owLength = 4;

  int count = 0;
  //set up received interrupt, but didnt setup when reading final bits
  for(count = 0; count <(owLength-1) ;count++){
    owSetUpRxIT_Expect(uartRxDataBuffer, 3);
  }
  get1BitRomLoop(&bsi, devices, 4);
  TEST_ASSERT_EQUAL(4, (*(bsi.romUid) & 0xf));
  TEST_ASSERT_EQUAL(2, lastDiscrepancy);
  TEST_ASSERT_EQUAL(FALSE, lastDeviceFlag);
  free(bsi.romUid);
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
  int devices[4][4] = {{1, 0, 0, 0},
                       {0, 1, 0, 0},
                       {0, 0, 1, 0},
                       {0, 0, 0, 1}};
  resetDeviceListTo1();

  BitSearchInformation bsi;
  initGet1BitRom(&bsi);
  bsi.romUid = (uint8_t*)malloc(OW_LENGTH);

  /*test prequisite*/
  lastDiscrepancy = 2;
  lastDeviceFlag = FALSE;
  *(bsi.romUid) = 0x04;

  owLength = 4;
  int count = 0;
  //set up received interrupt, but didnt setup when reading final bits
  for(count = 0; count <(owLength-1) ;count++){
    owSetUpRxIT_Expect(uartRxDataBuffer, 3);
  }

  get1BitRomLoop(&bsi, devices, 4);
  TEST_ASSERT_EQUAL(2, (*(bsi.romUid) & 0xf));
  TEST_ASSERT_EQUAL(1, lastDiscrepancy);
  TEST_ASSERT_EQUAL(FALSE, lastDeviceFlag);
  free(bsi.romUid);
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
  int devices[4][4] = {{1, 0, 0, 0},
                       {0, 1, 0, 0},
                       {0, 0, 1, 0},
                       {0, 0, 0, 1}};
  resetDeviceListTo1();

  BitSearchInformation bsi;
  initGet1BitRom(&bsi);
  bsi.romUid = (uint8_t*)malloc(OW_LENGTH);
  /*test prequisite*/
  lastDiscrepancy = 1;
  lastDeviceFlag = FALSE;
  *(bsi.romUid) = 0x02;
  owLength = 4;

  int count = 0;
  //set up received interrupt, but didnt setup when reading final bits
  for(count = 0; count <(owLength-1) ;count++){
    owSetUpRxIT_Expect(uartRxDataBuffer, 3);
  }

  get1BitRomLoop(&bsi, devices, 4);
  TEST_ASSERT_EQUAL(1, (*(bsi.romUid) & 0xf));
  TEST_ASSERT_EQUAL(0, lastDiscrepancy);
  TEST_ASSERT_EQUAL(TRUE, lastDeviceFlag);
  free(bsi.romUid);
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
   int devices[3][8] = {{0, 0, 1, 1, 0, 1, 0, 1},
                        {0, 1, 0, 1, 1, 0, 0, 1},
                        {0, 0, 1, 0, 0, 1, 1, 0}};
   resetDeviceListTo1();

   owLength = 8;
   BitSearchInformation bsi;
   initGet1BitRom(&bsi);
  //  clearGet1BitRom(&bsi);
   bsi.romUid = (uint8_t*)malloc(OW_LENGTH);

   int count = 0;
   //set up received interrupt, but didnt setup when reading final bits
   for(count = 0; count <(owLength-1) ;count++){
     owSetUpRxIT_Expect(uartRxDataBuffer, 3);
   }

   get1BitRomLoop(&bsi, devices, 3);
   TEST_ASSERT_EQUAL(0x26, (*(bsi.romUid)));
   TEST_ASSERT_EQUAL(1, lastDiscrepancy);
   TEST_ASSERT_EQUAL(FALSE, lastDeviceFlag);
   free(bsi.romUid);
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
    int devices[3][8] = {{0, 0, 1, 1, 0, 1, 0, 1},
                         {0, 1, 0, 1, 1, 0, 0, 1},
                         {0, 0, 1, 0, 0, 1, 1, 0}};
    resetDeviceListTo1();

    owLength = 8;
    BitSearchInformation bsi;
    initGet1BitRom(&bsi);
    bsi.romUid = (uint8_t*)malloc(OW_LENGTH);
    lastDeviceFlag = FALSE;
    lastDiscrepancy=1;
    *(bsi.romUid) = 0x26;
    int count = 0;
    //set up received interrupt, but didnt setup when reading final bits
    for(count = 0; count <(owLength-1) ;count++){
      owSetUpRxIT_Expect(uartRxDataBuffer, 3);
    }

    get1BitRomLoop(&bsi, devices, 3);
    TEST_ASSERT_EQUAL(0x59, (*(bsi.romUid)));
    TEST_ASSERT_EQUAL(3, lastDiscrepancy);
    TEST_ASSERT_EQUAL(FALSE, lastDeviceFlag);
    free(bsi.romUid);
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
     int devices[3][8] = {{0, 0, 1, 1, 0, 1, 0, 1},
                          {0, 1, 0, 1, 1, 0, 0, 1},
                          {0, 0, 1, 0, 0, 1, 1, 0}};
     resetDeviceListTo1();

     owLength = 8;
     BitSearchInformation bsi;
     // initGet1BitRom(&bsi);
     initGet1BitRom(&bsi);
     bsi.romUid = (uint8_t*)malloc(OW_LENGTH);
     lastDeviceFlag = FALSE;
     lastDiscrepancy=3;
     *(bsi.romUid) = 0x59;

     int count = 0;
     //set up received interrupt, but didnt setup when reading final bits
     for(count = 0; count <(owLength-1) ;count++){
       owSetUpRxIT_Expect(uartRxDataBuffer, 3);
     }

     get1BitRomLoop(&bsi, devices, 3);
     TEST_ASSERT_EQUAL(0x35, (*(bsi.romUid)));
     TEST_ASSERT_EQUAL(0, lastDiscrepancy);
     TEST_ASSERT_EQUAL(TRUE, lastDeviceFlag);
     free(bsi.romUid);
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
     int devices[3][16] = {{0, 0, 0, 0,  0, 1, 0, 1,  1, 1, 0, 0,  0, 1, 0, 1},
                           {0, 0, 0, 0,  1, 0, 1, 1,  1, 1, 0, 0,  0, 1, 0, 1},
                           {0, 0, 0, 0,  1, 0, 0, 1,  0, 0, 1, 0,  0, 1, 1, 0}};
     resetDeviceListTo1();

     owLength = 16;
     BitSearchInformation bsi;
     // initGet1BitRom(&bsi);
     initGet1BitRom(&bsi);
     bsi.romUid = (uint8_t*)malloc(OW_LENGTH);

     int count = 0;
     //set up received interrupt, but didnt setup when reading final bits
     for(count = 0; count <(owLength-1) ;count++){
       owSetUpRxIT_Expect(uartRxDataBuffer, 3);
     }

     targetSetupConfig(0xc5, &bsi);
     get1BitRomLoop(&bsi, devices, 3);
     TEST_ASSERT_EQUAL(10, lastDiscrepancy);
     TEST_ASSERT_EQUAL(FALSE, lastDeviceFlag);
     TEST_ASSERT_EQUAL(0xc5, (*(bsi.romUid)));
     TEST_ASSERT_EQUAL(0x05, (*(bsi.romUid + 1)));
     free(bsi.romUid);
   }

   /*Target Setup search (2/3)
   * this is the continue of the search on previous test, no targetSetupConfig() is called
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
   *            romUid[1] = 0xb
   */
   void test_targetSetupSearch_cont_givenAboveData_expect_dataTwo(void){
     int devices[3][16] = {{0, 0, 0, 0,  0, 1, 0, 1,  1, 1, 0, 0,  0, 1, 0, 1},
                           {0, 0, 0, 0,  1, 0, 1, 1,  1, 1, 0, 0,  0, 1, 0, 1},
                           {0, 0, 0, 0,  1, 0, 0, 1,  0, 0, 1, 0,  0, 1, 1, 0}};
     resetDeviceListTo1();

     owLength = 16;
     BitSearchInformation bsi;
     // initGet1BitRom(&bsi);
     initGet1BitRom(&bsi);
     bsi.romUid = (uint8_t*)malloc(OW_LENGTH);

     lastDiscrepancy = 10;
     lastFamilyDiscrepancy = 0;
     lastDeviceFlag = FALSE;
     bsi.romUid[0] = 0xc5; //family code
     bsi.romUid[1] = 0x05;

     int count = 0;
     //set up received interrupt, but didnt setup when reading final bits
     for(count = 0; count <(owLength-1) ;count++){
       owSetUpRxIT_Expect(uartRxDataBuffer, 3);
     }

     get1BitRomLoop(&bsi, devices, 3);
     TEST_ASSERT_EQUAL(0, lastDiscrepancy);
     TEST_ASSERT_EQUAL(TRUE, lastDeviceFlag);
     TEST_ASSERT_EQUAL(0xc5, (*(bsi.romUid)));
     TEST_ASSERT_EQUAL(0xb, (*(bsi.romUid + 1)));
     free(bsi.romUid);
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
 * devices: 0011 0100
 *          0x34
 *          0111  1011
 *          0x7b
 *          1011  1000
 *          0xb9
 *
 * RomUid to search : 0011 0100
 *                    0x34
 * --------------
 * MOCK data: idBit    :00001100
 *            cmp-id-bit:01010011
 *
 */
   void test_verify_device_given_device_same_with_ROMNO_EXPECT_same_ROMNO(void){
     int devices[3][8] = {{0, 0, 1, 1,  0, 1, 0, 0},
                           {0, 1, 1, 1,  1, 0, 1, 1},
                           {1, 0, 1, 1,  1, 0, 0, 0}};
     resetDeviceListTo1();

     owLength = 8;
     BitSearchInformation bsi;
     // initGet1BitRom(&bsi);
     initGet1BitRom(&bsi);
     bsi.romUid = (uint8_t*)malloc(OW_LENGTH);

     uint8_t romNumberToVerify = 0x34;
     verifyConfig(&romNumberToVerify, 1, &bsi);

     int count = 0;
     //set up received interrupt, but didnt setup when reading final bits
     for(count = 0; count <(owLength-1) ;count++){
       owSetUpRxIT_Expect(uartRxDataBuffer, 3);
     }

     get1BitRomLoop(&bsi, devices, 3);
     TEST_ASSERT_EQUAL(0x34, bsi.romUid[0]);
     free(bsi.romUid);
   }
/**
 * Verify certain romUid but given different romUid
 * ------------
 * Test on 8 bitPos
 * ------------
 * Given:
 * romUid to search: 1011 1111
 *                   0xbe
 *Given devices:    0110 1100
 *                  0x6d
 *                  1000 1110
 *                  0x8e
 *--------------------------------------------
 *MOCK data: idBit     :01110001
 *           cmpIdBit :00001110
 */
   void test_verify_device_given_device_different_from_ROMNO_expect_ROMNO_different(void){
     int devices[3][8] = {{0, 1, 1, 0,  1, 1, 0, 1},
                          {1, 0, 0, 0,  1, 1, 1, 0}};
     resetDeviceListTo1();

     owLength = 8;
     BitSearchInformation bsi;
     // initGet1BitRom(&bsi);
     initGet1BitRom(&bsi);
     bsi.romUid = (uint8_t*)malloc(OW_LENGTH);

     uint8_t romNumberToVerify = 0xbe;
     verifyConfig(&romNumberToVerify, 1, &bsi);

     //result should not be either 1 of the devices above, and should receive DEVICE_NOT_THERE flag
     bsi.bitReadType = DEVICE_NOT_THERE;
     get1BitRom(&bsi);

     TEST_ASSERT_EQUAL(0xbe, bsi.romUid[0]);
     TEST_ASSERT_NOT_EQUAL(0x8e, bsi.romUid[0]);
     TEST_ASSERT_NOT_EQUAL(0x6d, bsi.romUid[0]);
     TEST_ASSERT_EQUAL(TRUE, bsi.noDevice);
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
 */
 void test_FamilySkipSetup_Search_given_lastFamilyDiscrepancy_1(void){
   /*first search*/
   //----------------------------------------------------
   int devices[3][16] = {{0, 0, 1, 1,  1, 0, 0, 1,  0, 1, 1, 0,  1, 0, 1, 0},
                         {1, 1, 1, 0,  0, 1, 0, 1,  0, 1, 1, 0,  1, 0, 1, 0},
                         {0, 1, 1, 1,  1, 1, 1, 1,  1, 1, 0, 1,  0, 0, 0, 1}};
   resetDeviceListTo1();

   owLength = 16;
   BitSearchInformation bsi;
   initGet1BitRom(&bsi);
   bsi.romUid = (uint8_t*)malloc(OW_LENGTH);

   int count = 0;
   //set up received interrupt, but didnt setup when reading final bits
   for(count = 0; count <(owLength-1) ;count++){
     owSetUpRxIT_Expect(uartRxDataBuffer, 3);
   }

   get1BitRomLoop(&bsi, devices, 3);

   TEST_ASSERT_EQUAL(FALSE, lastDeviceFlag);
   TEST_ASSERT_EQUAL(0x6A, bsi.romUid[0]);
   TEST_ASSERT_EQUAL(1, lastFamilyDiscrepancy);
   /*second search*/
   //----------------------------------------------------
   resetDeviceListTo1();
   clearGet1BitRom(&bsi);
   searchDir = 0;
   familySkipConfig();

   for(count = 0; count <(owLength-1) ;count++){
     owSetUpRxIT_Expect(uartRxDataBuffer, 3);
   }

   get1BitRomLoop(&bsi, devices, 3);

   TEST_ASSERT_EQUAL(0x7f, bsi.romUid[1]);
   TEST_ASSERT_EQUAL(0xd1, bsi.romUid[0]);
 }

 /**
  * The 'FAMILY SKIP SETUP' operation sets the search state to skip all of the
  * devices that have the family code that was found in the previous search.
  *
  * given data:
  *            0011 1001 0110 1010       -> same family code   -->data no.1
  *            1110 0101 0110 1010       ---|                  -->data no.2
  *            0111 1111 1110 1010                             -->data no.3
  *
  * Flow of process:
  * ->first we perform 1wire first search, and it will found data no.1
  * ->perform familySkipSetupSearch and it will found data no.3 instead of data no.2
  *
  *                          ...(after the first search)
  *            0011 1001 0110 1010       -> same family code   -->data no.1
  *            1110 0101 0110 1010       ---|                  -->data no.2
  *            0111 1111 1110 1010                             -->data no.3 -------> this data will chosen in next search
  *                      ^
  *                      |
  *             lastFamilyDiscrepancy
  * 1wire firstSearch -------------> (1st)
  * ------------------
  * MOCK data: idBit     :0101 0110 1001 1000
  *           cmpIdBit  :0010 1001 0100 0011
  *           path taken  :0101 0110 1001 1000
  *
  * 1wire familySkipSetupSearch -------------> (2nd)
  */
void test_FamilySkipSetup_Search_given_lastFamilyDiscrepancy_8(void){
  int devices[3][16] = {{0, 0, 1, 1,  1, 0, 0, 1,  0, 1, 1, 0,  1, 0, 1, 0},
                        {1, 1, 1, 0,  0, 1, 0, 1,  0, 1, 1, 0,  1, 0, 1, 0},
                        {0, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 0,  1, 0, 1, 0 }};
  resetDeviceListTo1();

  owLength = 16;
  BitSearchInformation bsi;
  initGet1BitRom(&bsi);
  bsi.romUid = (uint8_t*)malloc(OW_LENGTH);

  int count = 0;
  //set up received interrupt, but didnt setup when reading final bits
  for(count = 0; count <(owLength-1) ;count++){
    owSetUpRxIT_Expect(uartRxDataBuffer, 3);
  }

  get1BitRomLoop(&bsi, devices, 3);

  TEST_ASSERT_EQUAL(FALSE, lastDeviceFlag);
  TEST_ASSERT_EQUAL(0x6A, bsi.romUid[0]);
  TEST_ASSERT_EQUAL(8, lastFamilyDiscrepancy);
  /*second search*/
  //----------------------------------------------------
  resetDeviceListTo1();
  clearGet1BitRom(&bsi);
  searchDir = 0;
  familySkipConfig();

  for(count = 0; count <(owLength-1) ;count++){
    owSetUpRxIT_Expect(uartRxDataBuffer, 3);
  }

  get1BitRomLoop(&bsi, devices, 3);

  TEST_ASSERT_EQUAL(0x7f, bsi.romUid[1]);
  TEST_ASSERT_EQUAL(0xea, bsi.romUid[0]);
}

/**
 * The 'FAMILY SKIP SETUP' operation sets the search state to skip all of the
 * devices that have the family code that was found in the previous search.
 *
 * given data:
 *            0011 1001 0110 1010       -> same family code   -->data no.1
 *            1110 0101 0110 1010       ---|                  -->data no.2
 *            0111 1111 1110 1010                             -->data no.3
 *
 * Flow of process:
 * ->first we perform 1wire first search, and it will found data no.1
 * ->perform familySkipSetupSearch and it will found data no.3 instead of data no.2
 *
 *                          ...(after the first search)
 *            0011 1001 0110 1010       -> same family code   -->data no.1
 *            1110 0101 0110 1010       ---|                  -->data no.2
 *            0111 1111 1111 1010                             -->data no.3 -------> this data will chosen in next search
 *                         ^
 *                         |
 *                lastFamilyDiscrepancy
 * 1wire firstSearch -------------> (1st)
 * ------------------
 * MOCK data: idBit     :0101 0110 1001 1000
 *           cmpIdBit  :0010 1001 0100 0011
 *           path taken  :0101 0110 1001 1000
 *
 * 1wire familySkipSetupSearch -------------> (2nd)
 */
void test_FamilySkipSetup_Search_given_lastFamilyDiscrepancy_5(void){
 int devices[3][16] = {{0, 0, 1, 1,  1, 0, 0, 1,  0, 1, 1, 0,  1, 0, 1, 0},
                       {1, 1, 1, 0,  0, 1, 0, 1,  0, 1, 1, 0,  1, 0, 1, 0},
                       {0, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 0, 1, 0 }};
 resetDeviceListTo1();

 owLength = 16;
 BitSearchInformation bsi;
 initGet1BitRom(&bsi);
 bsi.romUid = (uint8_t*)malloc(OW_LENGTH);

 int count = 0;
 //set up received interrupt, but didnt setup when reading final bits
 for(count = 0; count <(owLength-1) ;count++){
   owSetUpRxIT_Expect(uartRxDataBuffer, 3);
 }

 get1BitRomLoop(&bsi, devices, 3);

 TEST_ASSERT_EQUAL(FALSE, lastDeviceFlag);
 TEST_ASSERT_EQUAL(0x6A, bsi.romUid[0]);
 TEST_ASSERT_EQUAL(5, lastFamilyDiscrepancy);
 /*second search*/
 //----------------------------------------------------
 resetDeviceListTo1();
 clearGet1BitRom(&bsi);
 searchDir = 0;
 familySkipConfig();

 for(count = 0; count <(owLength-1) ;count++){
   owSetUpRxIT_Expect(uartRxDataBuffer, 3);
 }

 get1BitRomLoop(&bsi, devices, 3);

 TEST_ASSERT_EQUAL(0x7f, bsi.romUid[1]);
 TEST_ASSERT_EQUAL(0xfa, bsi.romUid[0]);
}




/**
 * test macro in specify in .h file
 */
 void test_GET_CURRENT_BIT_IN_ROM(void){
   BitSearchInformation *bsi ;
   bsi = (BitSearchInformation*)malloc(sizeof(BitSearchInformation));
   bsi->romUid = malloc(8);
   bsi->romByteNum = 1;
   bsi->byteMask = 0x04;
   *(bsi->romUid + 1) = 0x55;
   TEST_ASSERT_EQUAL(1, GET_CURRENT_BIT_IN_ROM(bsi));
   free(bsi->romUid);
   free(bsi);
 }

/**
 * test macro in specify in .h file
 * Initialize
 * ----------------
 * romUid[1] = 0 1 0 1 0 1 0 1
 * byteMask = 0 0 0 0 0 0 1 0
 * -----------------
 * result
 * romUid[1] = 0 1 0 1 0 1 1 1
 */
 void test_SET_ROM_BIT(void){
   BitSearchInformation *bsi ;
   bsi = (BitSearchInformation*)malloc(sizeof(BitSearchInformation));
   bsi->romUid = malloc(8);
   bsi->romByteNum = 1;
   bsi->byteMask = 0x02;
   *(bsi->romUid + 1) = 0x55;
   TEST_ASSERT_EQUAL(0x57, SET_ROM_BIT(bsi));
   free(bsi->romUid);
   free(bsi);
 }

 /**
  * test macro in specify in .h file
  * Initialize
  * ----------------
  * romUid[1] = 0 1 0 1 0 1 0 1
  * byteMask = 0 0 0 0 0 1 0 0
  * -----------------
  * result
  * romUid[1] = 0 1 0 1 0 0 0 1
  */
 void test_RESET_ROM_BIT(void){
   BitSearchInformation *bsi ;
   bsi = (BitSearchInformation*)malloc(sizeof(BitSearchInformation));
   bsi->romUid = malloc(8);
   bsi->romByteNum = 1;
   bsi->byteMask = 0x04;
   *(bsi->romUid + 1) = 0x55;
   TEST_ASSERT_EQUAL(0x51, RESET_ROM_BIT(bsi));
   free(bsi->romUid);
   free(bsi);
 }
/**
 * test macro in specify in .h file
 */
 void test_UPDATE_LAST_FAMILY_DISCREPANCY_given_lastZero_lessThan_FAMILYCODE(void){
   BitSearchInformation *bsi ;
   bsi = (BitSearchInformation*)malloc(sizeof(BitSearchInformation));
   bsi->lastZero = 3;
   lastFamilyDiscrepancy = 0;
   UPDATE_LAST_FAMILY_DISCREPANCY(bsi);
   TEST_ASSERT_EQUAL(3, lastFamilyDiscrepancy);
   free(bsi);
 }

 /**
  * test macro in specify in .h file
  */
 void test_UPDATE_LAST_FAMILY_DISCREPANCY_given_lastZero_greaterThan_FAMILYCODE(void){
   BitSearchInformation *bsi ;
   bsi = (BitSearchInformation*)malloc(sizeof(BitSearchInformation));
   bsi->lastZero = 9;
   lastFamilyDiscrepancy = 0;
   UPDATE_LAST_FAMILY_DISCREPANCY(bsi);
   TEST_ASSERT_EQUAL(0, lastFamilyDiscrepancy);
   free(bsi);
 }

 /**
  * test macro in specify in .h file
  */
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

 /**
  * test macro in specify in .h file
  */
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
 * test macro in specify in .h file
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
   int searchDir = 0;
   BitSearchInformation *bsi ;
   bsi = (BitSearchInformation*)malloc(sizeof(BitSearchInformation));
   bsi->lastZero = 0;
   bsi->idBitNumber = 65;
   owLength = 64;
   RESET_IF_COMPLETED_BIT_SEARCHING(bsi, searchDir);
   TEST_ASSERT_EQUAL(TRUE, lastDeviceFlag);
   TEST_ASSERT_EQUAL(0, bsi->lastZero);
   TEST_ASSERT_EQUAL(TRUE, bsi->searchResult);
   TEST_ASSERT_EQUAL(0, lastDiscrepancy);
   TEST_ASSERT_EQUAL(1, bsi->idBitNumber);
   free(bsi);
 }


 /**
  * test macro in specify in .h file
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
    int searchDir = 0;
    BitSearchInformation *bsi ;
    bsi = (BitSearchInformation*)malloc(sizeof(BitSearchInformation));
    bsi->idBitNumber = 65;
    owLength = 64;
    bsi->lastZero = 3;
    lastDeviceFlag = FALSE;
    RESET_IF_COMPLETED_BIT_SEARCHING(bsi, searchDir);
    TEST_ASSERT_EQUAL(FALSE, lastDeviceFlag);
    TEST_ASSERT_EQUAL(0, bsi->lastZero);
    TEST_ASSERT_EQUAL(TRUE, bsi->searchResult);
    TEST_ASSERT_EQUAL(3, lastDiscrepancy);
    TEST_ASSERT_EQUAL(1, bsi->idBitNumber);
    free(bsi);
  }

/**
 * test macro in specify in .h file
 * Test search process not yet complete
 * given: bsi->idBitNumber = 63
 *                owLength = 63
 * Expect: NO CHANGES
 */
  void test_RESET_IF_COMPLETED_BIT_SEARCHING_given_BitNumber_smallerThan_owLength(void){
    int searchDir = 0;
    BitSearchInformation *bsi ;
    bsi = (BitSearchInformation*)malloc(sizeof(BitSearchInformation));
    bsi->idBitNumber = 63;
    owLength = 63;
    bsi->lastZero = 3;
    lastDeviceFlag = FALSE;
    bsi->searchResult = FALSE;
    lastDiscrepancy = 0;
    owSetUpRxIT_Expect(uartRxDataBuffer, 3);
    RESET_IF_COMPLETED_BIT_SEARCHING(bsi, searchDir);
    TEST_ASSERT_EQUAL(FALSE, lastDeviceFlag);
    TEST_ASSERT_EQUAL(3, bsi->lastZero);
    TEST_ASSERT_EQUAL(FALSE, bsi->searchResult);
    TEST_ASSERT_EQUAL(0, lastDiscrepancy);
    free(bsi);
  }
