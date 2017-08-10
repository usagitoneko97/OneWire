#include "search.h"
#include "onewireio.h"
#include "owvariable.h"

/**
 * store 1 byte of data in a
 * @param data         1 byte of data that need to store
 * @param numberOfByte total number of byte of device rom number
 */
void stack_dataBuffer_64(uint8_t data, int numberOfByte){
  RomDataBuffer[bufferDeviceNumber][bufferByteNumber++] = data;
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
      RomDataBuffer[i][j] = 0;
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
  LastDiscrepancy = 0;
  LastDeviceFlag=FALSE;
  LastFamilyDiscrepancy = 0;
  int i;
  for(i=0;i<8;i++)
    ROM_NO[i] = 0;
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
  LastDiscrepancy = 0;
  LastDeviceFlag=FALSE;
  LastFamilyDiscrepancy = 0;
  int i;
  for(i=0;i<8;i++)
    ROM_NO[i] = 0;
  return _bitSearch(numberOfByte);
}

InnerVAR_OW processOWData(InnerVAR_OW innerVAR_OW){
  innerVAR_OW.id_bit = Read();
  innerVAR_OW.cmp_id_bit = Read();
  volatile int i = 0;
  i++;
  if(innerVAR_OW.id_bit == 1 && innerVAR_OW.cmp_id_bit == 1){  //no devices
    innerVAR_OW.noDevice = TRUE;
    return innerVAR_OW;
  }
  else{
    if(innerVAR_OW.id_bit != innerVAR_OW.cmp_id_bit){
      innerVAR_OW.search_direction = innerVAR_OW.id_bit;
    }
    else{
      if(innerVAR_OW.id_bit_number == LastDiscrepancy){
        innerVAR_OW.search_direction = 1;
      }
      else if(innerVAR_OW.id_bit_number > LastDiscrepancy){
        innerVAR_OW.search_direction = 0;
      }
      else{
        innerVAR_OW.search_direction = ((ROM_NO[innerVAR_OW.rom_byte_num] & innerVAR_OW.rom_byte_mask)>0);  //if there is "1" on any bit, load 1 to search_direction
      }
      if(!innerVAR_OW.search_direction){
        innerVAR_OW.last_zero = innerVAR_OW.id_bit_number;
        if(innerVAR_OW.last_zero<9){
          LastFamilyDiscrepancy = innerVAR_OW.last_zero;
        }

      }
    }
    if(innerVAR_OW.search_direction == 1){
      ROM_NO[innerVAR_OW.rom_byte_num] |= innerVAR_OW.rom_byte_mask; //set the current bit to be 1
      Write(1);
    }
    else{
      ROM_NO[innerVAR_OW.rom_byte_num] &= ~innerVAR_OW.rom_byte_mask; //set current bit to be 0
      Write(0);
    }


    //preparation for next bit search
    innerVAR_OW.id_bit_number++;
    innerVAR_OW.rom_byte_mask <<=1;

  }
  return innerVAR_OW;
}

int _bitSearch(int numberOfByte){
  InnerVAR_OW innerVAR_OW;

  if(!LastDeviceFlag){

    /*Initialize inner variables*/
    innerVAR_OW.id_bit_number = 1;
    innerVAR_OW.last_zero = 0;
    innerVAR_OW.rom_byte_num = 0;
    innerVAR_OW.rom_byte_mask = 1;
    innerVAR_OW.search_result = 0;
    innerVAR_OW.noDevice = FALSE;
    crc8 = 0;

    do{
        innerVAR_OW = processOWData(innerVAR_OW);
        //checking of a complete byte

        if(innerVAR_OW.rom_byte_mask == 0){
          stack_dataBuffer_64(ROM_NO[innerVAR_OW.rom_byte_num],numberOfByte);
          innerVAR_OW.rom_byte_mask = 1;
          innerVAR_OW.rom_byte_num++;
        }
        if(innerVAR_OW.noDevice == TRUE)
          break;
  }while(innerVAR_OW.rom_byte_num<numberOfByte);
    //done searching
    //if successful
    if(innerVAR_OW.id_bit_number > (numberOfByte<<3)){
    LastDiscrepancy = innerVAR_OW.last_zero;
    if(LastDiscrepancy == 0){
      LastDeviceFlag = TRUE;
    }
    innerVAR_OW.search_result = TRUE;
    }
    //no device found (break from id_bit_number = cmp_id_bit =1)
  }
    /*last device flag is true*/
    if(!innerVAR_OW.search_result){
      LastDiscrepancy = 0;
      LastDeviceFlag = FALSE;
      LastFamilyDiscrepancy = 0;
      innerVAR_OW.search_result = FALSE;
    }
    return innerVAR_OW.search_result;
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
    ROM_NO[0] = familyCode;
    for (i = 1; i < 8; i++)
      ROM_NO[i] = 0;
    LastDiscrepancy = 64;
    LastFamilyDiscrepancy = 0;
    LastDeviceFlag = FALSE;
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
   LastDiscrepancy = LastFamilyDiscrepancy;
   LastFamilyDiscrepancy = 0;

   // check for end of list
   if (LastDiscrepancy == 0)
      LastDeviceFlag = TRUE;
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
  LastDiscrepancy = 64;
  LastFamilyDiscrepancy = 0;
  LastDeviceFlag = FALSE;
  int i;
  for(i = 0;i<Bytelength;i++)
    ROM_NO[i] = *(romNumberToVerify+i);
}
