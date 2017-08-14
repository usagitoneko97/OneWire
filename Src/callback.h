#ifndef _CALLBACK_H
#define _CALLBACK_H
#include "owcompletesearch.h"
#include "linkedlist.h"


#define CALL(x)   if(x != NULL) (x)
void registerCallback(void (callBack)(Event*), LinkedList *list);
#endif // _CALLBACK_H
