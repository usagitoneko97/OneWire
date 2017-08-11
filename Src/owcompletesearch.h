#ifndef _OWCOMPLETESEARCH_H
#define _OWCOMPLETESEARCH_H
#include <stdint.h>

#define SEND_ZERO		0x0
#define SEND_ONE		0xff
#define RESET     0

#define SET_BIT(REG, BIT)     ((REG) |= (BIT))
//#define SENDF0_DATA  {SEND_ONE, SEND_ONE, SEND_ONE,SEND_ONE, SEND_ZERO, SEND_ZERO, SEND_ZERO, SEND_ZERO}
//uint8_t sendF0_txData[] = {SEND_1, SEND_1, SEND_1,SEND_1, SEND_0, SEND_0, SEND_0, SEND_0};


typedef enum {
  DEVICE_AVAILABLE = 0,
  DEVICE_NA = 1
}deviceAvail;

typedef enum{
  RESET_OW =0,
  REPLY = 1,
}EventType;


typedef struct EventStruct Event;
struct EventStruct {
  EventType eventType;
  void *data;
  void (*commandFunction)(Event*);
  int byteLength ;
};

typedef struct OwData OwData;
struct OwData {
  int idBit, cmpIdBit;
  uint8_t uartRxVal;
};
void OW_Tx_SendArray(uint8_t* data, int length);

void initRomSearching(Event* evt, void* owdata);
void resetOw(Event *evt);
void romSearch(Event *evt);

int isOwDeviceAvail(Event *evt);
int owHandler(Event *evt);

#endif // _OWCOMPLETESEARCH_H
