#include <stdio.h>
#include <string.h>
#include "linkedlist.h"
#include <stdlib.h>

void ListInit(LinkedList *list){
  list->head = NULL;
  list->tail = NULL;
  list->len = 0;

}

void ListAddEmptyLinkedList (LinkedList *list, Item *item){
  list->head = item;
  list->tail = item;
  list->len = 1;
  item->next = NULL;
}

void ListAddLinkedList(LinkedList *list, Item *item){
  if(list->head==NULL){
    //empty list
    list->head = item;
    list->tail = item;
    list->len = 1;
    item->next = NULL;
  }
  else{
    list->tail->next = item;
    item->next = NULL;
    list->tail = item;
    list->len++;

  }
}

Item* ListRemoveFirst(LinkedList *list){
  Item *temp;
  if(list->head == NULL){
    return NULL;
  }
  else{
    if(list->head == list->tail){
      //only 1 data
      ListInit(list);
    }
    else{
      temp = list->head;
      list->head = list->head->next;
      list->len--;
      //TODO free the deleted list
    }
  }
}
Item* ListRemoveLinkedListByName(char* name, LinkedList *list){
  //preserve the head

  Item *prevL = NULL;
  Item *currL = list->head;
  if(list->head == NULL){
    return NULL;
  }
    while(strcmp(((Student*)currL->data)->name, name) != 0){
      // prevL = currL;
      // currL = currL->next;
    prevL = currL;
    currL = currL->next;  //move to next item to search
    if(currL == NULL){
      //no data found
      return NULL;
    }
  }
      //succesfully found the name
      //----------------------------
      //currL = data to be deleted
    if(currL == NULL){
      //wrong input of name
      return NULL;
    }
    else
    {
      if(prevL == NULL){
        //data to be deleted is head
        ListRemoveFirst(list);
      }
      else if(currL == list->tail){
        //data to be deletd is tail
        list->tail = prevL;
        prevL->next = NULL;
        list->len--;
      }

      else{
        //normal deletion
        prevL->next = currL->next;
        list->len--;
      }

    }
}

void pushList(LinkedList *list, Item *listToAdd){
  if(list->head == NULL){
    ListAddEmptyLinkedList(list, listToAdd);
  }
  else{
    listToAdd->next = list->head;
    list->head = listToAdd;
    list->len ++;
  }
}

int dummy1(){
  return 23;
}
