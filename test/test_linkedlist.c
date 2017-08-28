#include "unity.h"
#include "linkedlist.h"
#include "callback.h"
Student ali = {
  "ali",
  23,
  72.35,
  1.78
};

Item itemAli = {
  (Item *)342523452,  //next
  (void *)&ali       //data
};

Student baba = {
  "baba",
  22,
  71.35,
  2.78
};

Item itemBaba = {
  (Item *)3425152,  //next
  (void *)&baba       //data
};

Student abu = {
  "abu",
  25,
  76.35,
  2.58
};

Item itemAbu = {
  (Item *)3425152,  //next
  (void *)&abu       //data
};


void setUp(void)
{
}

void tearDown(void)
{
}

void test_Listlinit_ensure_initialize_to_NULL_and_0(void){
  LinkedList list = {
    (Item*)-1,
    (Item*)-1,
    10};

  listInit(&list);
  TEST_ASSERT_NULL(list.head);
  TEST_ASSERT_NULL(list.tail);
  TEST_ASSERT_EQUAL(0, list.len);
}

void test_ListAdd_with_emptyList_(void){
  LinkedList list;
  Student ali = {
    "Ali",
    23,
    72.35,
    1.78
  };

  Item itemAli = {
    (Item *)342523452,  //next
    (void *)&ali       //data
  };
  listInit(&list);
  listAddEmptyLinkedList(&list, &itemAli);
  TEST_ASSERT_EQUAL_PTR(&itemAli, list.head);
  TEST_ASSERT_EQUAL_PTR(&itemAli, list.tail);
  TEST_ASSERT_EQUAL(1, list.len);
  TEST_ASSERT_NULL(itemAli.next);
}

void test_listAddLinkedList_2_addwith_1(void){
  LinkedList list;
  list.head = &itemAli;
  list.tail = &itemAli;
  list.len = 1;
  itemAli.next = NULL;
  itemBaba.next = (Item *)-1; //item needed to add

  listAddLinkedList(&list, &itemBaba);

  TEST_ASSERT_EQUAL_PTR(&itemAli, list.head);
  TEST_ASSERT_EQUAL_PTR(&itemBaba, list.tail);
  TEST_ASSERT_EQUAL(2, list.len);

  TEST_ASSERT_EQUAL_PTR(&itemBaba, itemAli.next);

  TEST_ASSERT_NULL(itemBaba.next);

  TEST_ASSERT_EQUAL_PTR(&baba, itemBaba.data);
}

void test_listAddLinkedList_3_addwith_1(void){
  LinkedList list;
  list.head = &itemAli;
  list.tail = &itemBaba;
  list.len = 2;
  itemAli.next = &itemBaba;
  itemBaba.next = NULL; //item needed to add
  listAddLinkedList(&list, &itemAbu);

  TEST_ASSERT_EQUAL_PTR(&itemAli, list.head);
  TEST_ASSERT_EQUAL_PTR(&itemAbu, list.tail);
  TEST_ASSERT_EQUAL(3, list.len);

  TEST_ASSERT_EQUAL_PTR(&itemBaba, itemAli.next);
  TEST_ASSERT_EQUAL_PTR(&itemAbu, itemBaba.next);

  TEST_ASSERT_NULL(itemAbu.next);

}

void test_removeFirstList_ali_abu_baba_remove_ali_expect_abu_baba(void){
  LinkedList list;
  listInit(&list);
  listAddLinkedList(&list, &itemAli);
  listAddLinkedList(&list, &itemAbu);
  listAddLinkedList(&list, &itemBaba);
  listRemoveFirst(&list);
  TEST_ASSERT_EQUAL_PTR(&itemAbu, list.head);
  TEST_ASSERT_EQUAL_PTR(&itemBaba, list.tail);
  TEST_ASSERT_EQUAL_PTR(&itemBaba, itemAbu.next);
  TEST_ASSERT_NULL(itemBaba.next);
  TEST_ASSERT_EQUAL(2, list.len);
}

void test_removeFirstList_ali_abu_remove_ali_expect_abu(void){
  LinkedList list;
  listInit(&list);
  listAddLinkedList(&list, &itemAli);
  listAddLinkedList(&list, &itemAbu);
  listRemoveFirst(&list);
  TEST_ASSERT_EQUAL_PTR(&itemAbu, list.head);
  TEST_ASSERT_EQUAL_PTR(&itemAbu, list.tail);
  TEST_ASSERT_NULL(itemAbu.next);
  TEST_ASSERT_EQUAL(1, list.len);
}

void test_removeFirstList_ali_remove_ali_expect_NULL(void){
  LinkedList list;
  listInit(&list);
  listAddLinkedList(&list, &itemAli);
  listRemoveFirst(&list);
  TEST_ASSERT_NULL(list.head);
  TEST_ASSERT_NULL(list.tail);
  TEST_ASSERT_EQUAL(0, list.len);
}

/*
no data attempts to remove data
*/
void test_removeLinkedList_NULL_remove_ali_expect_NULL(void){
  LinkedList list;
  listInit(&list);
  listRemoveLinkedListByName("ali", &list);

  TEST_ASSERT_NULL(list.head);
  TEST_ASSERT_NULL(list.tail);
}

/*
wrong input of name
*/
void test_removeLinkedList_abu_baba_remove_wrong_name_expect_abu_baba(void){
  LinkedList list;
  listInit(&list);
  listAddLinkedList(&list, &itemAbu);
  listAddLinkedList(&list, &itemBaba);
  listRemoveLinkedListByName("wrong_name", &list);

  TEST_ASSERT_EQUAL_PTR(&itemAbu, list.head);
  TEST_ASSERT_EQUAL_PTR(&itemBaba, list.tail);
  TEST_ASSERT_EQUAL_PTR(&itemBaba, itemAbu.next);
  TEST_ASSERT_NULL(itemBaba.next);
  TEST_ASSERT_EQUAL(2, list.len);
}

/*
1 data remove data
*/
void test_removeLinkedList_ali_remove_ali_expect_NULL(void){
  LinkedList list;
  listInit(&list);
  listAddLinkedList(&list, &itemAli);
  listRemoveLinkedListByName("ali", &list);

  TEST_ASSERT_NULL(list.head);
  TEST_ASSERT_NULL(list.tail);
  TEST_ASSERT_EQUAL(0, list.len);
}

/*
*2 data remove data at first
*/
void test_removeLinkedList_ali_abu_remove_ali_expect_abu(void){
  LinkedList list;
  listInit(&list);
  listAddLinkedList(&list, &itemAli);
  listAddLinkedList(&list, &itemAbu);
  listRemoveLinkedListByName("ali", &list);

  TEST_ASSERT_EQUAL_PTR(&itemAbu, list.head);
  TEST_ASSERT_EQUAL_PTR(&itemAbu, list.tail);
  TEST_ASSERT_NULL(itemAbu.next);
  TEST_ASSERT_EQUAL(1, list.len);
}

/*
*2 data remove data at last
*/
void test_removeLinkedList_ali_abu_remove_abu_expect_ali(void){
  LinkedList list;
  listInit(&list);
  listAddLinkedList(&list, &itemAli);
  listAddLinkedList(&list, &itemAbu);
  listRemoveLinkedListByName("abu", &list);

  TEST_ASSERT_EQUAL_PTR(&itemAli, list.head);
  TEST_ASSERT_NULL(itemAli.next);

  TEST_ASSERT_EQUAL(1, list.len);
}


/*
*3 data remove data at first
*/
void test_removeLinkedList_removeFirst_ali_abu_baba_remove_ali_expect_abu_baba(void){
  LinkedList list;
  listInit(&list);
  listAddLinkedList(&list, &itemAli);
  listAddLinkedList(&list, &itemAbu);
  listAddLinkedList(&list, &itemBaba);
  listRemoveLinkedListByName("ali", &list);

  TEST_ASSERT_EQUAL_PTR(&itemAbu, list.head);
  TEST_ASSERT_EQUAL_PTR(&itemBaba, list.tail);
  TEST_ASSERT_EQUAL_PTR(&itemBaba, itemAbu.next);
  TEST_ASSERT_EQUAL(2, list.len);

}

/*
*3 data remove data in the middle
*/
void test_removeLinkedList_ali_abu_baba_remove_abu_expect_ali_baba(void){
  LinkedList list;
  listInit(&list);
  listAddLinkedList(&list, &itemAli);
  listAddLinkedList(&list, &itemAbu);
  listAddLinkedList(&list, &itemBaba);
  listRemoveLinkedListByName("abu", &list);
  TEST_ASSERT_EQUAL_PTR(&itemBaba, itemAli.next); //Ali next is baba
  TEST_ASSERT_EQUAL_PTR(&itemAli, list.head);
  TEST_ASSERT_EQUAL_PTR(&itemBaba, list.tail);
  TEST_ASSERT_EQUAL(2, list.len);
}

/*
*3 data remove data at last
*/
void test_removeLinkedList_removeFirst_ali_abu_baba_remove_baba_expect_ali_abu(void){
  LinkedList list;
  listInit(&list);
  listAddLinkedList(&list, &itemAli);
  listAddLinkedList(&list, &itemAbu);
  listAddLinkedList(&list, &itemBaba);
  listRemoveLinkedListByName("baba", &list);

  TEST_ASSERT_EQUAL_PTR(&itemAli, list.head);
  TEST_ASSERT_EQUAL_PTR(&itemAbu, list.tail);
  TEST_ASSERT_EQUAL_PTR(&itemAbu, itemAli.next);
  TEST_ASSERT_EQUAL(2, list.len);

}

void test_pushList_given_NULL__push_Ali_expect_itemAli(void){
  LinkedList list;
  listInit(&list);
  pushList(&list, &itemAli);
  TEST_ASSERT_EQUAL_PTR(&itemAli, list.head);
  TEST_ASSERT_EQUAL_PTR(&itemAli, list.tail);
  TEST_ASSERT_NULL(itemAli.next);
}

void test_pushList_given_ali_abu_expect_baba_ali_abu(void){
  LinkedList list;
  listInit(&list);
  listAddLinkedList(&list, &itemAli);
  listAddLinkedList(&list, &itemAbu);
  pushList(&list, &itemBaba);

  TEST_ASSERT_EQUAL_PTR(&itemBaba, list.head);
  TEST_ASSERT_EQUAL_PTR(&itemAbu, list.tail);
  TEST_ASSERT_EQUAL_PTR(&itemAli, itemBaba.next);
  TEST_ASSERT_EQUAL_PTR(&itemAbu ,itemAli.next);
  TEST_ASSERT_EQUAL(3, list.len);
}

void test_push2list_expect_ali_abu(void){
  LinkedList list;
  listInit(&list);
  pushList(&list, &itemAbu);
  pushList(&list, &itemAli);

  TEST_ASSERT_EQUAL_PTR(&itemAli, list.head);
  TEST_ASSERT_EQUAL_PTR(&itemAbu, list.tail);
  TEST_ASSERT_EQUAL_PTR(&itemAbu, itemAli.next);
  TEST_ASSERT_EQUAL(2, list.len);
}
