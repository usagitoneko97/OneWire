#include "unity.h"
#include "callback.h"
#include "linkedlist.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void someFunction(Event* evt){
  printf("success!!\n");
}
void somethingBetween(){

}
void anotherFunction(Event *evt){

}

void test_registerCallback_given_emptyList(void){
  LinkedList list;
  ListInit(&list);
  registerCallback(someFunction, &list);
  TEST_ASSERT_EQUAL_PTR(someFunction, ((TxRxCallbackList*)((list.head)->data))->txRxCallbackFuncP);

  Event evt;
  evt.evtType = UART_RX_SUCCESS;
}

void test_registerCallback_given_list_with_1_data(void){
  LinkedList list;
  ListInit(&list);
  /*registerCallback(anotherFunction, &list);
  registerCallback(someFunction, &list);*/

  TxRxCallbackList callBackList;
  callBackList.txRxCallbackFuncP = someFunction;
  Item item;
  item.data = (void*)&callBackList;
  pushList(&list, &item);

  TxRxCallbackList callBackList1;
  callBackList1.txRxCallbackFuncP = anotherFunction;
  Item item1;
  item1.data = (void*)&callBackList1;
  pushList(&list, &item1);

  Item *itemHead = list.head;
  TxRxCallbackList *callBackList5 = (TxRxCallbackList*)(itemHead->data);
  TEST_ASSERT_EQUAL_PTR(anotherFunction, callBackList5->txRxCallbackFuncP);
  TEST_ASSERT_NOT_NULL(list.head);
  TxRxCallbackList *callBackListNext = (TxRxCallbackList*)(itemHead->next->data);
  TEST_ASSERT_EQUAL(2,list.len);
  TEST_ASSERT_EQUAL_PTR(someFunction, callBackListNext->txRxCallbackFuncP);
  Event evt;
  evt.evtType = UART_RX_SUCCESS;
  callBackListNext->txRxCallbackFuncP(&evt);

}
