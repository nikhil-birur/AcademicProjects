#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#include "storage_mgr.h"
#include "dberror.h"
#include "test_helper.h"
#include "buffer_pool.h"



// Init buffer storage.
Buffer_Storage *initBufferStorage(char *pageFileName, int capacity) {
  SM_FileHandle *fh;
  Queue *pool = createQueue(capacity);
  Buffer_Storage *bs;
  bs = (Buffer_Storage *)malloc(sizeof(Buffer_Storage));
  bs->pool = pool;

  //Init hash, it's important when tesing in server end.
  int i;
  for (i = 0; i < 65535; i++) {
    bs->mapping[i] = NULL;
  }
  return bs;
}

Queue *createQueue(int capacity) {

  // printf("queue size %d\n", capacity);
  Queue* queue = (Queue *)malloc(sizeof(Queue));

  // Initialize queue as empty.
  queue->count = 0;
  queue->front = queue->rear = NULL;

  // The capacity of buffer pool.
  queue->q_capacity = capacity;
  queue->readIO = 0;
  queue->writeIO = 0;

  return queue;
}

Page_Frame* newPageFrame( int pageNum , BM_PageHandle *page)
{
    Page_Frame* temp = (Page_Frame *)malloc( sizeof( Page_Frame ) );
    // temp->pageNumber = pageNum;
    temp->pageHandle = page;
    temp->prev = temp->next = NULL;
    temp->is_dirty = FALSE;
    temp->fix_count = 1;
    temp->lastUsed = 0;
    return temp;
}

int isPoolFull(BM_BufferPool *bm) {
  Buffer_Storage *bs = (Buffer_Storage*)bm->mgmtData;
  Queue *q = bs->pool;
  if(q->count==q->q_capacity){
    // printf("in isPoolFull %d, %d\n", q->q_capacity, q->count);
    return 1;
  }
  else{
    return 0;
  }
}

int ReplacementFIFO(Queue *queue, Page_Frame **mapping, Page_Frame *removed, Page_Frame *added){
    // If all frames are full, remove the page at the rear
    if (queue->count == queue->q_capacity){

      int replaced;
      enQueue(queue, added);
      replaced = deQueue(queue);
      int index = mapping[replaced]->index;
      added->index = index;
      return replaced;
    }

    else {
      enQueue(queue, added);
    }
    return -1;
}


int ReplacementLRU(Queue *queue, Page_Frame **mapping, Page_Frame *removed, Page_Frame *added) {

    if (queue->count >= queue->q_capacity){

      int replaced;
      enQueue(queue, added);
      replaced = deQueue(queue);
      int index = mapping[replaced]->index;
      added->index = index;
      return replaced;
    }

    else {
      enQueue(queue, added);
    }
    return -1;

}


int enQueue(Queue *queue, Page_Frame *added) {

  if (queue->count == 0) {
    // printf("queue is empty\n");
    queue->rear = queue->front = added;
    added->index = 0;
  }
  else {
    queue->rear->next = added;
    added->prev = queue->rear;
    queue->rear = added;
    if (queue->count < queue->q_capacity){
      added->index = queue->count;
    }
  }
  queue->count++;
  return 1;
}


// display queue items.
int printQueueElement(Queue *queue) {
  printf("===pool====\n");
  Page_Frame *f = queue->front;
  int i;
  while(f) {
    printf("addresss is %p, pageNum is %d is_dirty=%d fix_count=%d index=%d\n", f, f->pageHandle->pageNum, f->is_dirty, f->fix_count, f->index);
    f = f->next;
  }
  printf("===pool end====\n");
  return 1;
}



// check if there is remove avaliable page frames, return null if buffer pool
// is busy.,
Page_Frame *checkRemoved(Queue *queue) {
  Page_Frame *removedAvaiable = NULL;
  Page_Frame *temp = queue->front;


  while(temp) {
    if (temp->fix_count == 0) {
      removedAvaiable = temp;
      break;
    }
    else
      temp = temp->next;
  }
  return removedAvaiable;
}


// check if current page frame is the front of queue.
int isFront(Queue *queue, Page_Frame *pf) {
  if (queue->front == pf) {
    return 1;
  }
  return 0;
}

// check if current page frame is the rear of queue.
int isRear(Queue *queue, Page_Frame *pf) {
  if (queue->rear == pf) {
    return 1;
  }
  return 0;
}

void removeFromQueue(Queue *queue, Page_Frame *pf) {
  if (isFront(queue, pf)){
    // printf("remove node from front, it's %d\n", pf->pageHandle->pageNum);

    queue->front = queue->front->next;
  }
  else if (isRear(queue, pf)){
    // printf("remove node from rear\n");
    queue->rear = queue->rear->prev;
  }
  else {
    // printf("removed node from middle\n");
    Page_Frame *temp = pf;

    temp->prev->next = pf->next;
    temp->next->prev = pf->prev;

    free(temp);

  }
}

int deQueue( Queue *queue )
{
    int removed;
    // queue is empty.
    if(queue->count == 0)
        return -1;

    // if the none of elements in queue has fix_count=0,
    // then buffer pool is busy, we can not replace elements.
    Page_Frame *removedPF;
    removedPF = checkRemoved(queue);
    if (removedPF == NULL) {
      printf("buffer is busy\n");
      return RC_BUFFER_BUSY;
    }
    else {
    // If this is the only node in list, then change front
      if (queue->count == 1) {
          removed = queue->rear->pageHandle->pageNum;
          //empty a queue.
          queue->front = NULL;
          queue->rear = NULL;
      }
      else
        removeFromQueue(queue, removedPF);
      // free(temp);
      queue->count--;
      return removedPF->pageHandle->pageNum;
  }

}
