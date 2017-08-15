#include <stdio.h>
#include <stdlib.h>
#include "callback.h"

void registerCallback(void (callBack)(Event*), LinkedList *list){
  //create TxRxCallbackList item
  //create item to associate TxRxCallbackList
  //add inside linkedlist
  TxRxCallbackList *callBackList = (TxRxCallbackList*)malloc(sizeof(TxRxCallbackList));
  callBackList->txRxCallbackFuncP = callBack;
  Item *item = (Item*)malloc(sizeof(Item));
  item->data = (void*)callBackList;
  pushList(list, item);
}

void unregisterCallback(LinkedList *list){
  ListRemoveFirst(list);
}

FuncP getCurrentCallback(LinkedList *list){
  Item *itemHead = list->head;
  TxRxCallbackList *callBackList5 = (TxRxCallbackList*)(itemHead->data);
  return callBackList5->txRxCallbackFuncP;
}
