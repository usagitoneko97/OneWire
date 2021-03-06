#include "unity.h"
#include "callback.h"
#include "linkedlist.h"
#include "callback.h"
void setUp(void)
{
}

void tearDown(void)
{
}

void someFunction(Event* evt){
}
void somethingBetween(){

}
void anotherFunction(Event *evt){

}

void finalFunction(Event *evt){

}

void test_registerCallback_given_emptyList(void){
  LinkedList list;
  listInit(&list);
  registerCallback(someFunction, &list);
  TEST_ASSERT_EQUAL_PTR(someFunction, ((TxRxCallbackList*)((list.head)->data))->txRxCallbackFuncP);

  Event evt;
  evt.evtType = UART_RX_SUCCESS;
}

void test_registerCallback_given_list_with_1_data(void){
  LinkedList list;
  listInit(&list);
  registerCallback(someFunction, &list);
  registerCallback(anotherFunction, &list);

  Item *itemHead = list.head;
  TxRxCallbackList *callBackList5 = (TxRxCallbackList*)(itemHead->data);
  TEST_ASSERT_EQUAL_PTR(anotherFunction, callBackList5->txRxCallbackFuncP);
  TEST_ASSERT_NOT_NULL(list.head);
  TxRxCallbackList *callBackListNext = (TxRxCallbackList*)(itemHead->next->data);
  TEST_ASSERT_EQUAL(2,list.len);
  TEST_ASSERT_EQUAL_PTR(someFunction, callBackListNext->txRxCallbackFuncP);
}

void test_unregisterCallback_given_list_with2_data_expect_1Data(void){
  LinkedList list;
  listInit(&list);
  registerCallback(someFunction, &list);
  registerCallback(anotherFunction, &list);

  unregisterCallback(&list);

  Item *itemHead = list.head;
  TxRxCallbackList *callBackList5 = (TxRxCallbackList*)(itemHead->data);
  TEST_ASSERT_EQUAL_PTR(someFunction, callBackList5->txRxCallbackFuncP);
}

void test_getCurrentCallback_given_1_data(void){
  LinkedList list;
  listInit(&list);
  registerCallback(someFunction, &list);
  FuncP tempFuncP = getCurrentCallback(&list);
  TEST_ASSERT_EQUAL(someFunction, tempFuncP);
}

void test_getCurrentCallback_given_NULL(void){
  LinkedList list;
  listInit(&list);
  FuncP tempFuncP = getCurrentCallback(&list);
  TEST_ASSERT_NULL(tempFuncP);
}

void test_given_register_then_unregister_then_register(){
  LinkedList list;
  listInit(&list);
  registerCallback(someFunction, &list);
  registerCallback(anotherFunction, &list);

  unregisterCallback(&list);
  registerCallback(finalFunction, &list);

  Item *itemHead = list.head;
  TxRxCallbackList *callBackList5 = (TxRxCallbackList*)(itemHead->data);
  TEST_ASSERT_EQUAL_PTR(finalFunction, callBackList5->txRxCallbackFuncP);

}
