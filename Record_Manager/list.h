#ifndef __LIST_H__
#define __LIST_H__

#include "dberror.h"


#define LISTCAPACITY 100

typedef struct ListNode {
	struct ListNode *next;
	struct ListNode *prev;
	void *value;
} ListNode;

typedef struct List {
	ListNode *head;
	ListNode *tail;

	int itemCount;
	int listCapacity;
	int isFull;
} List;


List *createList(void);
RC insert(List *l, void *item);
ListNode *popTail(List *l);
RC releaseList(List *l);
RC printList(List *l);
#endif
