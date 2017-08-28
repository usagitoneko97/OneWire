#ifndef _CALLBACK_H
#define _CALLBACK_H
#include "OwCompleteSearch.h"
#include "LinkedList.h"

typedef void (*FuncP)(Event*);
#define CALL(x)   if(x != NULL) (x)
void registerCallback(void (callBack)(Event*), LinkedList *list);
void unregisterCallback(LinkedList *list);
FuncP getCurrentCallback(LinkedList *list);
#endif // _CALLBACK_H
