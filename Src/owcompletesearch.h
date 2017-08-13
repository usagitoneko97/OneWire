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
  REPLY = 1,
  UART_FRAME_ERROR = 2,
  UART_TIMEOUT = 3,
  UART_RX_SUCCESS = 4,
  RESET_DEVICE_AVAILABLE = 5,
  RESET_DEVICE_NOT_AVAILABLE = 6,
  RESET_DEVICE_UNKNOWN_ERROR = 7,
  ROM_SEARCH_SUCCESSFUL = 8,
}EventType;


typedef struct EventStruct EventStruct;
struct EventStruct {
  EventType eventType;
  void *data;
  void (*commandFunction)(EventStruct*);
  int byteLength ;
};

typedef struct OwData OwData;
struct OwData {
  int idBit, cmpIdBit;
  uint8_t uartRxVal;
};

/*Generic Event*/
typedef struct Event{
  EventType evtType;
  void *data;
}Event;

/*uartTxRx Event data*/
typedef struct TxRxCpltEvData {
  uint8_t *uartRxVal;
  int length;
}TxRxCpltEvData;

/*Reset Ow Parameter*/
typedef enum{
  RESET_OW = 0,
  REPLY_OW = 1
}OwResetState;

typedef struct OwResetPrivate{
  OwResetState state;
}OwResetPrivate;

/*romSearching parameter*/
typedef struct BitSearchInformation BitSearchInformation;
struct BitSearchInformation {
  int idBitNumber;
  int lastZero, romByteNum, searchResult;
  int idBit, cmpIdBit;
  unsigned char searchDirection;
  unsigned char byteMask;
  int noDevice;
};

typedef enum {
  SEND_F0 = 0,
  ROM_SEARCHING = 1,
}RomSearchingState;

typedef struct RomSearchingPrivate {
  RomSearchingState state;
  uint8_t *romNo;
  BitSearchInformation bitSearchInformation;
}RomSearchingPrivate;

typedef struct RomSearchingEvData{
  uint8_t *romDataBuffer;
  int lastDeviceFlag;     //TODO change type to boolean
}RomSearchingEvData;

void OW_Tx_SendArray(uint8_t* data, int length);

void initRomSearching(EventStruct* evt, void* owdata);
void resetOw(EventStruct *evt);
void romSearch(EventStruct *evt);
void calcIdCmpId(uint8_t *uartRxVal, int *idBitNumber, int *cmpIdBitNumber);

int isOwDeviceAvail(EventStruct *evt);
int owHandler(EventStruct *evt);


void resetAndVerifyOw(Event *evt);
void initGetBitRom(RomSearchingPrivate *romSearchingPrivate);
void romSearching(Event *evt);
void updateSearch(RomSearchingPrivate *romSearchingPrivate);
void doRomSearch(Event *evt);
void clearGetRom(RomSearchingPrivate *romSearchingPrivate);
#endif // _OWCOMPLETESEARCH_H
