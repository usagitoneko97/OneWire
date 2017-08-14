#include "callback.h"
#include <stdio.h>

void registerCallback(void (callBack)(Event*), LinkedList *list){
  //create TxRxCallbackList item
  //create item to associate TxRxCallbackList
  //add inside linkedlist
  static TxRxCallbackList callBackList;
  callBackList.txRxCallbackFuncP = callBack;
  static Item item;
  item.data = (void*)&callBackList;
  pushList(list, &item);
}

void *unregisterCallback(){

}

void *getCurrentCallback(){

}
