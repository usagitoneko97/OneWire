#ifndef _LINKEDLIST_H
#define _LINKEDLIST_H


#include "owcompletesearch.h"

typedef struct Item Item;
typedef struct LinkedList LinkedList;
typedef struct Student Student;

struct Student{
  char name[256];
  int age;
  float weight;
  float height;
};

typedef struct TxRxCallbackList TxRxCallbackList;
  struct TxRxCallbackList{
    TxRxCallbackList *next;
    void (*txRxCallbackFuncP)(Event*);
    void *txRxdata;
};


struct Item{
  Item* next;
  void* data;
};

struct LinkedList{
  Item *head;
  Item *tail;
  int len;
};

void ListInit(LinkedList *list);
void ListAddEmptyLinkedList (LinkedList *list, Item *item);
void ListAddLinkedList(LinkedList *list, Item *item);
Item* ListRemoveFirst(LinkedList *list);
Item* ListRemoveLinkedListByName(char* name, LinkedList *list);
void pushList(LinkedList *list, Item *listToAdd);
int dummy1();

#endif // _LINKEDLIST_H
