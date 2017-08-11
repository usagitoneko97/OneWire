#ifndef _SEARCH_H
#define _SEARCH_H
#include <stdint.h>
typedef struct InnerVAR_OW InnerVAR_OW;
struct InnerVAR_OW {
  int idBitNumber;
  int lastZero, romByteNum, searchResult;
  int idBit, cmpIdBit;
  unsigned char searchDirection;
  unsigned char rom_byte_mask;
  int noDevice;
};
void stackDataBuffer64(uint8_t data, int numberOfByte);
void clearDataBuffer64();
int firstSearch();
InnerVAR_OW processOWData(InnerVAR_OW innerVAR_OW);
int bitSearch();
int _firstSearch(int numberOfByte);
int _bitSearch(int numberOfByte);
void targetSetupSearch(unsigned char familyCode);
void familySkipSetupSearch();
void verify(unsigned char *romNumberToVerify, int Bytelength);

#endif // _SEARCH_H
