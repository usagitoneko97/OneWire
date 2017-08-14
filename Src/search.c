#include "search.h"
#include "onewireio.h"
#include "owvariable.h"
#include <stdio.h>
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
 * Initialize to initial condition and perform bit searching
 * @return status of bitSearch
 */
int firstSearch() {
  clearDataBuffer64();
  lastDiscrepancy = 0;
  lastDeviceFlag=FALSE;
  lastFamilyDiscrepancy = 0;
  int i;
  for(i=0;i<8;i++)
    romNo[i] = 0;
  return bitSearch();
}
/**
 * initialize to initial condition and perform bitsearching with specific
 * number of byte of rom
 * @param  numberOfByte number of byte of rom number to be search
 * @return              status of bitSearch
 */
int _firstSearch(int numberOfByte) {
  clearDataBuffer64();
  lastDiscrepancy = 0;
  lastDeviceFlag=FALSE;
  lastFamilyDiscrepancy = 0;
  int i;
  for(i=0;i<8;i++)
    romNo[i] = 0;
  return _bitSearch(numberOfByte);
}

/**
 * read an idBitNumber and cmpIdBit, and write to 1 wire device as well as
 * romNo bit by bit
 * @param  bitSearchInformation structure that contain all the information
 *                              about the bit searching
 */
 void process1BitRom(BitSearchInformation *bitSearchInformation){
   bitSearchInformation->idBit = Read();
   bitSearchInformation->cmpIdBit = Read();
   volatile int i = 0;
   i++;
   if(bitSearchInformation->idBit == 1 && bitSearchInformation->cmpIdBit == 1){  //no devices
     bitSearchInformation->noDevice = TRUE;
   }
   else{
     if(bitSearchInformation->idBit != bitSearchInformation->cmpIdBit){
       bitSearchInformation->searchDirection = bitSearchInformation->idBit;
     }
     else{
       if(bitSearchInformation->idBitNumber == lastDiscrepancy){
         bitSearchInformation->searchDirection = 1;
       }
       else if(bitSearchInformation->idBitNumber > lastDiscrepancy){
         bitSearchInformation->searchDirection = 0;
       }
       else{
         bitSearchInformation->searchDirection = ((romNo[bitSearchInformation->romByteNum] & bitSearchInformation->byteMask)>0);  //if there is "1" on any bit, load 1 to searchDirection
       }
       if(!bitSearchInformation->searchDirection){
         bitSearchInformation->lastZero = bitSearchInformation->idBitNumber;
         if(bitSearchInformation->lastZero<9){
           lastFamilyDiscrepancy = bitSearchInformation->lastZero;
         }

       }
     }
     if(bitSearchInformation->searchDirection == 1){
       romNo[bitSearchInformation->romByteNum] |= bitSearchInformation->byteMask; //set the current bit to be 1
       write(1);
     }
     else{
       romNo[bitSearchInformation->romByteNum] &= ~bitSearchInformation->byteMask; //set current bit to be 0
       write(0);
     }


     //preparation for next bit search
     bitSearchInformation->idBitNumber++;
     bitSearchInformation->byteMask <<=1;


   }
 }



void get1BitRom(BitSearchInformation *bsi){
  int searchDir;
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
  write(searchDir);

  //preparation for next bit search
  bsi->idBitNumber++;
  UPDATE_ROM_BYTE_MASK(bsi);
  RESET_IF_COMPLETED_BIT_SEARCHING(bsi);
}


/**
 * custom bitsearch that takes in number of byte
 * @param  numberOfByte number of byte of the rom device
 * @return the search result
 */
int _bitSearch(int numberOfByte){
  BitSearchInformation bitSearchInformation;

  if(!lastDeviceFlag){

    /*Initialize inner variables*/
    bitSearchInformation.idBitNumber = 1;
    bitSearchInformation.lastZero = 0;
    bitSearchInformation.romByteNum = 0;
    bitSearchInformation.byteMask = 1;
    bitSearchInformation.searchResult = 0;
    bitSearchInformation.noDevice = FALSE;
    crc8 = 0;

    do{
        process1BitRom(&bitSearchInformation);
        //checking of a complete byte

        if(bitSearchInformation.byteMask == 0){
          stackDataBuffer64(romNo[bitSearchInformation.romByteNum],numberOfByte);
          bitSearchInformation.byteMask = 1;
          bitSearchInformation.romByteNum++;
        }
        if(bitSearchInformation.noDevice == TRUE)
          break;
  }while(bitSearchInformation.romByteNum<numberOfByte);
    //done searching
    //if successful
    if(bitSearchInformation.idBitNumber > (numberOfByte<<3)){
    lastDiscrepancy = bitSearchInformation.lastZero;
    if(lastDiscrepancy == 0){
      lastDeviceFlag = TRUE;
    }
    bitSearchInformation.searchResult = TRUE;
    }
    //no device found (break from idBitNumber = cmpIdBit =1)
  }
  /*last device flag is true*/
  if(!bitSearchInformation.searchResult){
    lastDiscrepancy = 0;
    lastDeviceFlag = FALSE;
    lastFamilyDiscrepancy = 0;
    bitSearchInformation.searchResult = FALSE;
  }
  return bitSearchInformation.searchResult;
}

void clearGet1BitRom(BitSearchInformation *bsi){
  bsi->lastZero = 0;
  bsi->romByteNum = 0;
  bsi->byteMask = 1;
  bsi->searchResult = 0;
  bsi->noDevice = FALSE;
  bsi->idBitNumber = 1;
}

int bitSearch(){
  return _bitSearch(8);
}

/**
 * The 'TARGET SETUP' operation is a way to preset the search state to first
 * find a particular family type
 * @param familyCode 1 byte of family code. Usually is the lsb of the rom numbers.
 * @NOTE this function only modify the state to perform specific search variations
 * and does not perform search itself
 */
void targetSetupSearch(unsigned char familyCode){
    int i;
    romNo[0] = familyCode;
    for (i = 1; i < 8; i++)
      romNo[i] = 0;
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
void familySkipSetupSearch()
{
   // set the Last discrepancy to last family discrepancy
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
void verify(unsigned char *romNumbers, int Bytelength){
  lastDiscrepancy = 64;
  lastFamilyDiscrepancy = 0;
  lastDeviceFlag = FALSE;
  int i;
  for(i = 0;i<Bytelength;i++)
    romNo[i] = *(romNumbers+i);
}
