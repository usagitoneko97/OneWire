#ifndef _CALLBACK_H
#define _CALLBACK_H
#include "owcompletesearch.h"
#include "linkedlist.h"

typedef void (*FuncP)(Event*);
#define CALL(x)   if(x != NULL) (x)
void registerCallback(void (callBack)(Event*), LinkedList *list);
void unregisterCallback(LinkedList *list);
FuncP getCurrentCallback();
#endif // _CALLBACK_H
