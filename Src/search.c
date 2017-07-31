#include "search.h"
#include "onewireio.h"
#include "owvariable.h"

void stack_dataBuffer_64(uint8_t data, int numberOfByte){
  RomDataBuffer[bufferDeviceNumber][bufferByteNumber++] = data;
  if(bufferByteNumber == numberOfByte){
    bufferDeviceNumber++;
    bufferByteNumber = 0;
  }
}

void clearDataBuffer_64(){
  bufferDeviceNumber = 0;
  bufferByteNumber = 0;
  int i, j;
  while(i <MAX_OWDEVICE){
    for(j = 0;j<8;j++){
      RomDataBuffer[i][j] = 0;
    }
    i++;
  }
}

int firstSearch() {
  clearDataBuffer_64();
  LastDiscrepancy = 0;
  LastDeviceFlag=FALSE;
  LastFamilyDiscrepancy = 0;
  int i;
  for(i=0;i++;i<8)
    ROM_NO[i] = 0;
  return bitSearch();
}
int _firstSearch(int numberOfByte) {
  clearDataBuffer_64();
  LastDiscrepancy = 0;
  LastDeviceFlag=FALSE;
  LastFamilyDiscrepancy = 0;
  int i;
  for(i=0;i++;i<8)
    ROM_NO[i] = 0;
  return _bitSearch(numberOfByte);
}

InnerVAR_OW processOWData(InnerVAR_OW innerVAR_OW){
  innerVAR_OW.id_bit = Read();
  innerVAR_OW.cmp_id_bit = Read();
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
    if(innerVAR_OW.search_direction == 1)
      ROM_NO[innerVAR_OW.rom_byte_num] |= innerVAR_OW.rom_byte_mask; //set the current bit to be 1
    else
      ROM_NO[innerVAR_OW.rom_byte_num] &= ~innerVAR_OW.rom_byte_mask; //set current bit to be 0
    //write()

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

    //Write(0xF0);
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
  _bitSearch(8);
}

int targetSetupSearch(unsigned char familyCode){
    int i;
    ROM_NO[0] = familyCode;
    for (i = 1; i < 8; i++)
      ROM_NO[i] = 0;
    LastDiscrepancy = 64;
    LastFamilyDiscrepancy = 0;
    LastDeviceFlag = FALSE;
}

void familySkipSetupSearch()
{
   // set the Last discrepancy to last family discrepancy
   LastDiscrepancy = LastFamilyDiscrepancy;
   LastFamilyDiscrepancy = 0;

   // check for end of list
   if (LastDiscrepancy == 0)
      LastDeviceFlag = TRUE;
}
