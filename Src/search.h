#ifndef _SEARCH_H
#define _SEARCH_H
#include "owcompletesearch.h"
#include <stdint.h>

#define FAMILY_CODE_RANGE             8
#define GET_CURRENT_BIT_IN_ROM(bsi) (((bsi)->romNo[(bsi)->romByteNum] &       \
                                   bsi->byteMask) > 0)
#define SET_ROM_BIT(bsi)            ((bsi)->romNo[(bsi)->romByteNum] |=       \
                                   bsi->byteMask)
#define RESET_ROM_BIT(bsi)          ((bsi)->romNo[(bsi)->romByteNum] &=       \
                                   ~bsi->byteMask)
#define UPDATE_LAST_FAMILY_DISCREPANCY(bsi)                                    \
                                   if((bsi)->lastZero <= FAMILY_CODE_RANGE)  \
                                     lastFamilyDiscrepancy = (bsi)->lastZero;
#define UPDATE_ROM_BYTE_MASK(bsi)    if(((bsi)->byteMask <<= 1) == 0){        \
                                       (bsi)->byteMask = 1;                   \
                                       (bsi)->romByteNum++;                   \
                                      }
#define RESET_IF_COMPLETED_BIT_SEARCHING(bsi)                                 \
                                     if(bsi->idBitNumber > OW_LENGTH){        \
                                       lastDiscrepancy = bsi->lastZero;       \
                                       if(lastDiscrepancy == 0){              \
                                         lastDeviceFlag = TRUE;               \
                                       }                                      \
                                       clearGet1BitRom(bsi);                  \
                                       bsi->searchResult = TRUE;              \
                                     }

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
