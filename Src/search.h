#ifndef _SEARCH_H
#define _SEARCH_H
#include <stdint.h>
typedef struct BitSearchInformation BitSearchInformation;
struct BitSearchInformation {
  int idBitNumber;
  int lastZero, romByteNum, searchResult;
  int idBit, cmpIdBit;
  unsigned char searchDirection;
  unsigned char byteMask;
  int noDevice;
};
void stackDataBuffer64(uint8_t data, int numberOfByte);
void clearDataBuffer64();
int firstSearch();
void process1BitRom(BitSearchInformation* bitSearchInformation);
int bitSearch();
int _firstSearch(int numberOfByte);
int _bitSearch(int numberOfByte);
void targetSetupSearch(unsigned char familyCode);
void familySkipSetupSearch();
void verify(unsigned char *romNumberToVerify, int Bytelength);

#endif // _SEARCH_H
