#ifndef _SEARCH_H
#define _SEARCH_H
#include "owcompletesearch.h"
#include <stdint.h>

void stackDataBuffer64(uint8_t data, int numberOfByte);
void clearDataBuffer64();
int firstSearch();
void process1BitRom(BitSearchInformation* bitSearchInformation);
void get1BitRom(BitSearchInformation *bsi);
int bitSearch();
int _firstSearch(int numberOfByte);
int _bitSearch(int numberOfByte);
void targetSetupSearch(unsigned char familyCode);
void familySkipSetupSearch();
void verify(unsigned char *romNumberToVerify, int Bytelength);
void clearGet1BitRom(BitSearchInformation *bsi);



#endif // _SEARCH_H
