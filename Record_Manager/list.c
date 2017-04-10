#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#include "dberror.h"
#include "dt.h"
#include "list.h"
#include "tables.h"


List *createList(void) {
	List *l;

	if ((l = (List *)malloc(sizeof(List))) == NULL)
		return NULL;

	l->head = l->tail = NULL;
	l->itemCount = 0;
	l->listCapacity = LISTCAPACITY;
	l->isFull = false;
	return l;
}


RC insert(List *l, void *item) {
	ListNode *node;
	RID *id = (RID *)item;

	if ( (node = (ListNode *)malloc(sizeof(ListNode))) == NULL ) {
		// error inserting new node.
		return -1;
	}
	node->value = item;

	if (l->itemCount == 0) {
		l->head = l->tail = node;
		node->next = NULL;
		node->prev = NULL;
	}
	else {
		l->tail->next = node;
		node->prev = l->tail;
		l->tail = node;
		node->next = NULL;
	}
	l->itemCount++;
	return RC_OK;
}

ListNode *popTail(List *l) {
	// node ready to be popped out.
	ListNode *node;

	if (l->itemCount == 0) {
		// no more item to be poped out.
		printf("List is empty\n");
		exit(0);
	}

	node = l->tail;

	if (l->itemCount == 1) {
		l->head = l->tail = NULL;
	}

	else {
		l->tail = node->prev;
		l->tail->next = NULL;
	}
	l->itemCount--;
	return node;
}

RC printList(List *l) {
	if (l->itemCount == 0) {
		// printf("List is empty\n");
		return -1;
	}
	ListNode *node = l->head;
	RID *id;
	printf("### List (size=%d)###\n", l->itemCount);
	while(node != NULL) {
		id = (RID *)node->value;
		printf("page=%d, slot=%d\n", id->page, id->slot);
		node = node->next;
	}
	printf("### End ###\n");
	return RC_OK;
}

RC releaseList(List *l) {
	ListNode *current, *next;

	current = l->head;

	while (current != NULL) {
		next = current->next;
		free(current);
		current = next;
	}
	return RC_OK;
}
