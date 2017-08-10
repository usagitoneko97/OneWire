#include "search.h"
#include "onewireio.h"
#include "owvariable.h"

/**
 * store 1 byte of data in a
 * @param data         1 byte of data that need to store
 * @param numberOfByte total number of byte of device rom number
 */
void stack_dataBuffer_64(uint8_t data, int numberOfByte){
  romDataBuffer[bufferDeviceNumber][bufferByteNumber++] = data;
  if(bufferByteNumber == numberOfByte){
    bufferDeviceNumber++;
    bufferByteNumber = 0;
  }
}

/**
 * clear data buffer
 */
void clearDataBuffer_64(){
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
  clearDataBuffer_64();
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
  clearDataBuffer_64();
  lastDiscrepancy = 0;
  lastDeviceFlag=FALSE;
  lastFamilyDiscrepancy = 0;
  int i;
  for(i=0;i<8;i++)
    romNo[i] = 0;
  return _bitSearch(numberOfByte);
}

InnerVAR_OW processOWData(InnerVAR_OW innerVAR_OW){
  innerVAR_OW.idBit = Read();
  innerVAR_OW.cmpIdBit = Read();
  volatile int i = 0;
  i++;
  if(innerVAR_OW.idBit == 1 && innerVAR_OW.cmpIdBit == 1){  //no devices
    innerVAR_OW.noDevice = TRUE;
    return innerVAR_OW;
  }
  else{
    if(innerVAR_OW.idBit != innerVAR_OW.cmpIdBit){
      innerVAR_OW.searchDirection = innerVAR_OW.idBit;
    }
    else{
      if(innerVAR_OW.idBitNumber == lastDiscrepancy){
        innerVAR_OW.searchDirection = 1;
      }
      else if(innerVAR_OW.idBitNumber > lastDiscrepancy){
        innerVAR_OW.searchDirection = 0;
      }
      else{
        innerVAR_OW.searchDirection = ((romNo[innerVAR_OW.romByteNum] & innerVAR_OW.rom_byte_mask)>0);  //if there is "1" on any bit, load 1 to searchDirection
      }
      if(!innerVAR_OW.searchDirection){
        innerVAR_OW.lastZero = innerVAR_OW.idBitNumber;
        if(innerVAR_OW.lastZero<9){
          lastFamilyDiscrepancy = innerVAR_OW.lastZero;
        }

      }
    }
    if(innerVAR_OW.searchDirection == 1){
      romNo[innerVAR_OW.romByteNum] |= innerVAR_OW.rom_byte_mask; //set the current bit to be 1
      write(1);
    }
    else{
      romNo[innerVAR_OW.romByteNum] &= ~innerVAR_OW.rom_byte_mask; //set current bit to be 0
      write(0);
    }


    //preparation for next bit search
    innerVAR_OW.idBitNumber++;
    innerVAR_OW.rom_byte_mask <<=1;

  }
  return innerVAR_OW;
}

int _bitSearch(int numberOfByte){
  InnerVAR_OW innerVAR_OW;

  if(!lastDeviceFlag){

    /*Initialize inner variables*/
    innerVAR_OW.idBitNumber = 1;
    innerVAR_OW.lastZero = 0;
    innerVAR_OW.romByteNum = 0;
    innerVAR_OW.rom_byte_mask = 1;
    innerVAR_OW.searchResult = 0;
    innerVAR_OW.noDevice = FALSE;
    crc8 = 0;

    do{
        innerVAR_OW = processOWData(innerVAR_OW);
        //checking of a complete byte

        if(innerVAR_OW.rom_byte_mask == 0){
          stack_dataBuffer_64(romNo[innerVAR_OW.romByteNum],numberOfByte);
          innerVAR_OW.rom_byte_mask = 1;
          innerVAR_OW.romByteNum++;
        }
        if(innerVAR_OW.noDevice == TRUE)
          break;
  }while(innerVAR_OW.romByteNum<numberOfByte);
    //done searching
    //if successful
    if(innerVAR_OW.idBitNumber > (numberOfByte<<3)){
    lastDiscrepancy = innerVAR_OW.lastZero;
    if(lastDiscrepancy == 0){
      lastDeviceFlag = TRUE;
    }
    innerVAR_OW.searchResult = TRUE;
    }
    //no device found (break from idBitNumber = cmpIdBit =1)
  }
    /*last device flag is true*/
    if(!innerVAR_OW.searchResult){
      lastDiscrepancy = 0;
      lastDeviceFlag = FALSE;
      lastFamilyDiscrepancy = 0;
      innerVAR_OW.searchResult = FALSE;
    }
    return innerVAR_OW.searchResult;
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
