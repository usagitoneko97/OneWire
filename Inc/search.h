#ifndef _SEARCH_H
#define _SEARCH_H
#include "owcompletesearch.h"
#include <stdint.h>

extern int searchDir;
#define FAMILY_CODE_RANGE             8
#define GET_CURRENT_BIT_IN_ROM(bsi) (((bsi)->romUid[(bsi)->romByteNum] &       \
                                   bsi->byteMask) > 0)
#define SET_ROM_BIT(bsi)            ((bsi)->romUid[(bsi)->romByteNum] |=       \
                                   bsi->byteMask)
#define RESET_ROM_BIT(bsi)          ((bsi)->romUid[(bsi)->romByteNum] &=       \
                                   ~bsi->byteMask)
#define UPDATE_LAST_FAMILY_DISCREPANCY(bsi)                                    \
                                   if((bsi)->lastZero <= FAMILY_CODE_RANGE)  \
                                     lastFamilyDiscrepancy = (bsi)->lastZero;
#define UPDATE_ROM_BYTE_MASK(bsi)    if(((bsi)->byteMask <<= 1) == 0){        \
                                       (bsi)->byteMask = 1;                   \
                                       (bsi)->romByteNum++;                   \
                                      }
#define RESET_IF_COMPLETED_BIT_SEARCHING(bsi, searchDir)                      \
                                     if(bsi->idBitNumber > OW_LENGTH){        \
                                       lastDiscrepancy = bsi->lastZero;       \
                                       if(lastDiscrepancy == 0){              \
                                         lastDeviceFlag = TRUE;               \
                                       }                                      \
                                       clearGet1BitRom(bsi);                  \
                                       bsi->searchResult = TRUE;              \
                                     }										                    \
									                   else{									                  \
										                  owSetUpRxIT(uartRxDataBuffer, 3);	      \
										                  write(searchDir);					              \
									                   }										                    \

void stackDataBuffer64(uint8_t data, int numberOfByte);
void clearDataBuffer64();

void get1BitRom(BitSearchInformation *bsi);

void targetSetupSearch(unsigned char familyCode);
void familySkipSetupSearch();
void verify(unsigned char *romNumberToVerify, int Bytelength);
void clearGet1BitRom(BitSearchInformation *bsi);

void targetSetupConfig(uint8_t familyCode, BitSearchInformation *bsi);
void verifyConfig(uint8_t *romNumbers, int byteLength, BitSearchInformation *bsi);
void familySkipConfig();


#endif // _SEARCH_H
