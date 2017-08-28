#include "search.h"
#include "onewireio.h"
#include "owvariable.h"
#include <stdio.h>

int searchDir = 0;
/**
 * store 1 byte of data in a
 * @param data         1 byte of data that need to store
 * @param numberOfByte total number of byte of device rom number
 * @test test_stackDataBuffer64
 */
void stackDataBuffer64(uint8_t data, int numberOfByte){
  romDataBuffer[bufferDeviceNumber][bufferByteNumber++] = data;
  if(bufferByteNumber == numberOfByte){
    bufferDeviceNumber++;
    bufferByteNumber = 0;
  }
}

/**
 * clear data buffer
 */
void clearDataBuffer64(){
  bufferDeviceNumber = 0;
  bufferByteNumber = 0;
  int i = 0, j;
  while(i <MAX_OWDEVICE){
    for(j = 0;j<8;j++){
      romDataBuffer[i][j] = 0;
    }
    i++;
  }
}




/**
 * read an idBitNumber and cmpIdBit, and write to 1 wire device as well as
 * romUid bit by bit
 * @param  bitSearchInformation structure that contain all the information
 *                              about the bit searching
 */
void get1BitRom(BitSearchInformation *bsi){
  switch (bsi->bitReadType) {
    case BIT_0:
      searchDir = 0;
      break;
    case BIT_1:
      searchDir = 1;
      break;
    case BIT_CONFLICT:
      if(bsi->idBitNumber == lastDiscrepancy){
        searchDir = 1;
      } else if(bsi->idBitNumber > lastDiscrepancy){
        searchDir = 0;
      } else{
        searchDir = GET_CURRENT_BIT_IN_ROM(bsi);
      }

      if(searchDir == 0){
        // Record the last zero encountered when conflicting
        bsi->lastZero = bsi->idBitNumber;
        // Update last family discrepancy if in family code range
        UPDATE_LAST_FAMILY_DISCREPANCY(bsi);
      }
      break;
    case DEVICE_NOT_THERE:
      bsi->noDevice = TRUE;
      return;
  }
  searchDir == 1? SET_ROM_BIT(bsi):RESET_ROM_BIT(bsi);


  //preparation for next bit search
  bsi->idBitNumber++;
  UPDATE_ROM_BYTE_MASK(bsi);
  RESET_IF_COMPLETED_BIT_SEARCHING(bsi, searchDir);
}




void clearGet1BitRom(BitSearchInformation *bsi){
  bsi->lastZero = 0;
  bsi->romByteNum = 0;
  bsi->byteMask = 1;
  bsi->searchResult = 0;
  bsi->noDevice = FALSE;
  bsi->idBitNumber = 1;
}


/**
 * The 'TARGET SETUP' operation is a way to preset the search state to first
 * find a particular family type
 * @param familyCode 1 byte of family code. Usually is the lsb of the rom numbers.
 * @NOTE this function only modify the state to perform specific search variations
 * and does not perform search itself
 */
void targetSetupConfig(uint8_t familyCode, BitSearchInformation *bsi){
  int i;
  *(bsi->romUid) = familyCode;
  for (i = 1; i < 8; i++)
    *(bsi->romUid + i)  = 0;
  lastDiscrepancy = 64;
  lastFamilyDiscrepancy = 0;
  lastDeviceFlag = FALSE;
}

/**
 * The 'FAMILY SKIP SETUP' operation sets the search state to skip all of the
 * devices that have the family code that was found in the previous search.
 *
 * @NOTE this function only modify the state to perform specific search variations
 *       and does not perform search itself
 */
void familySkipConfig(){
  lastDiscrepancy = lastFamilyDiscrepancy;
  lastFamilyDiscrepancy = 0;

  // check for end of list
  if (lastDiscrepancy == 0)
     lastDeviceFlag = TRUE;
}

/**
 * verifies if a device with a known ROM is currently connected to the 1-Wire
 *
 * @param romNumbers        rom number(s) to verify with
 * @param Bytelength        length in byte of romNumbers
 * @NOTE this function only modify the state to perform specific search variations
 * and does not perform search itself
 */
void verifyConfig(uint8_t *romNumbers, int byteLength, BitSearchInformation *bsi){
  lastDiscrepancy = 64;
  lastFamilyDiscrepancy = 0;
  lastDeviceFlag = FALSE;
  int i;
  for(i = 0;i<byteLength;i++)
    bsi->romUid[i] = *(romNumbers+i);
}
