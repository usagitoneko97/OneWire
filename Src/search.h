#ifndef _SEARCH_H
#define _SEARCH_H
#include <stdint.h>
typedef struct InnerVAR_OW InnerVAR_OW;
struct InnerVAR_OW {
  int id_bit_number;
  int last_zero, rom_byte_num, search_result;
  int id_bit, cmp_id_bit;
  unsigned char search_direction;
  unsigned char rom_byte_mask;
  int noDevice;
};
void stack_dataBuffer_64(uint8_t data, int numberOfByte);
void clearDataBuffer_64();
int firstSearch();
InnerVAR_OW processOWData(InnerVAR_OW innerVAR_OW);
int bitSearch();
int _firstSearch(int numberOfByte);
int _bitSearch(int numberOfByte);
int targetSetupSearch(unsigned char familyCode);
void familySkipSetupSearch();


#endif // _SEARCH_H
