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


// Init buffer manager.
RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName,
		  const int numPages, ReplacementStrategy strategy,
		  void *stratData){

    bm->pageFile = (char *)pageFileName;
    bm->numPages = numPages;
    bm->strategy = strategy;


		// Init buffer pool.
    bm->mgmtData = (Buffer_Storage *)initBufferStorage(bm->pageFile, numPages);

  return RC_OK;
}


// shutdown and free memory.
RC shutdownBufferPool(BM_BufferPool *const bm) {
	SM_FileHandle fHandle;
  Buffer_Storage *bs = (Buffer_Storage*)bm->mgmtData;
  Queue *q = bs -> pool;
  CHECK(openPageFile(bm->pageFile, &fHandle))
  Page_Frame *temp = q->front;


	// Check all dirty page frame and write contents to disk.
  while(temp!=NULL){
	if(temp-> fix_count>0){
		return RC_CANNOT_SHUTDOWN;
	}
	if(temp->is_dirty == true ){
		CHECK(writeBlock(temp-> pageHandle->pageNum, &fHandle, temp-> pageHandle->data));
		temp->is_dirty = FALSE;
		q->writeIO++;
	}

	temp = temp->next;
  }
  CHECK(closePageFile(&fHandle))

	//free mapping.
	int i;
	for (i = 0; i < 10000; i++) {
		bs->mapping[i] = NULL;
	}

  // free(bs);
  q = NULL;
  bs = NULL;

	// printf("shutdown\n");
  return RC_OK;
}

RC forceFlushPool(BM_BufferPool *const bm) {
	SM_FileHandle fHandle;
	Buffer_Storage *bs = (Buffer_Storage*)bm->mgmtData;
	Queue *q = bs -> pool;
	CHECK(openPageFile(bm->pageFile, &fHandle))
	Page_Frame *temp = q->front;
	// Loop through queue to find dirty page frames.
	while(temp!=NULL){
		// printf("temp->%d\n", temp->pageHandle->pageNum );
		if(temp->is_dirty == TRUE && temp-> fix_count == 0 ){
			CHECK(writeBlock(temp-> pageHandle->pageNum, &fHandle, temp-> pageHandle->data));
			temp->is_dirty = FALSE;
			q->writeIO++;

		}
		temp = temp->next;
	}
	CHECK(closePageFile( &fHandle));
	return RC_OK;
}


RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page) {
	SM_FileHandle fHandle;
	Buffer_Storage *bs = (Buffer_Storage*)bm->mgmtData;
	Queue *q = bs -> pool;
	CHECK(openPageFile(bm->pageFile, &fHandle));

	// if the pageNum is greater than the total number of pages in page file,
	// increase total number of page file.
	ensureCapacity(page->pageNum, &fHandle);
	writeBlock(page->pageNum, &fHandle, page->data);
	q->writeIO++;
	CHECK(closePageFile( &fHandle));
	return RC_OK;
}


RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page,
	    const PageNumber pageNum) {

	// pageNum is found in mapping table.
	// printf("## pinPage is %d##\n", pageNum);
	Buffer_Storage *bs = (Buffer_Storage *)bm->mgmtData;

  SM_FileHandle fh;
  SM_PageHandle ph;
  ph = (SM_PageHandle) malloc(PAGE_SIZE);
	BM_PageHandle *p = MAKE_PAGE_HANDLE();
	int replaced;


	// removed : should be removed page frame.
	// added: newly added page frame.
	Page_Frame *removed;
	Page_Frame *added = NULL;
	Queue *pool = bs->pool;


	// read from mapping.
	if (bs->mapping[pageNum]) {
		if (bm->strategy == RS_LRU){
				if (pool->count >1) {
					removeFromQueue(bs->pool, bs->mapping[pageNum]);
					Page_Frame *renewed = newPageFrame(pageNum, bs->mapping[pageNum]->pageHandle);

					renewed->index = bs->mapping[pageNum]->index;
					enQueue(bs->pool, renewed);
					bs->mapping[pageNum] = renewed;

				}
		}
		else {
			bs->mapping[pageNum]->fix_count++;
		}
		page->data = (bs->mapping[pageNum])->pageHandle->data;
		page->pageNum = pageNum;

		return RC_OK;
	}
	// read from page file.
	else {
    openPageFile(bm->pageFile, &fh);
    if (fh.totalNumPages < pageNum) {
      ensureCapacity(pageNum+1, &fh);
    }
    readBlock(pageNum, &fh, ph);

		pool->readIO++;

		// BM_PageHandle *newPageHandle = MAKE_PAGE_HANDLE();


		// assign pageNum and content to pageHandle.
		p->pageNum = pageNum;
		p->data = ph;



		// update mapping.
		added = newPageFrame(pageNum, p);
		bs->mapping[pageNum] = added;
    closePageFile(&fh);
	}


	if (bm->strategy == RS_FIFO) {
		// 'replaced' is the pageNum of the page which is being removed.
		replaced = ReplacementFIFO(bs->pool, bs->mapping, removed, added);
	}
	else if (bm->strategy == RS_LRU)  {
		// 'replaced' is the pageNum of the page which is being removed.
		replaced = ReplacementLRU(bs->pool, bs->mapping, removed, added);
	}

	// if replace is -1, which means buffer pool is not full, no need for replacement.
	if (replaced != -1){
		if ((bs->mapping[replaced])->is_dirty){


			// if repalced page frame is dirty, write content to disk.
			SM_FileHandle fHandle;
			CHECK(openPageFile(bm->pageFile, &fHandle));
			CHECK(writeBlock(replaced, &fHandle, bs->mapping[replaced]->pageHandle->data));
			CHECK(closePageFile( &fHandle));
			pool->writeIO++;

		}
		else {
			// not dirty.
			printf("current page(page=%d) is not dirty\n", pageNum);
		}

		// remove mapping of repalced page frame.
		if (bs->mapping[replaced]->fix_count == 0) {
			bs->mapping[replaced] = NULL;
		}

	}


	// update pageHandle.
	page->pageNum = added->pageHandle->pageNum;
	page->data = added->pageHandle->data;

	return RC_OK;

}


RC markDirty(BM_BufferPool * const bm, BM_PageHandle * const page)
{
	// mark page frame as dirty.
	Buffer_Storage *bs = (Buffer_Storage *)bm->mgmtData;

	Page_Frame *pf;
	BM_PageHandle *ph = MAKE_PAGE_HANDLE();
	ph->data = page->data;
	ph->pageNum = page->pageNum;

	if (bs->mapping[page->pageNum]) {
		// printf("mark pageNum %d dirty\n", page->pageNum);
		pf = bs->mapping[page->pageNum];
		pf->pageHandle = ph;
		pf->is_dirty = true;
		return RC_OK;
	}
	else {
		// printf("gose here error\n");
		return -1;
	}
}

RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page){


	// unpin a page and decrease fix_count.
	Buffer_Storage *bs = (Buffer_Storage *)bm->mgmtData;
	Page_Frame *pf;
	BM_PageHandle *ph = MAKE_PAGE_HANDLE();
	ph->data = page->data;
	ph->pageNum = page->pageNum;
	Queue *q = bs->pool;

	// find page frame from mapping, takes O(1).
	if (bs->mapping[page->pageNum]) {
		bs->mapping[page->pageNum]->fix_count--;
		return RC_OK;
	}
	else {
		// error.
		// printf("gose here error\n");
		return -1;
	}
	return RC_OK;
}

// Statistics functions.
PageNumber *getFrameContents (BM_BufferPool *const bm) {
	Buffer_Storage *bs = (Buffer_Storage *)bm->mgmtData;
  PageNumber *arrnumP1 = (PageNumber *)malloc(bm->numPages * sizeof(PageNumber));
	Queue *pool = bs->pool;
	Page_Frame *temp = pool->front;
	printQueueElement(pool);

	int i=0;
	// if buffer pool is not full, added "NO_PAGE" to empty buffer pool.
	if (pool->count < pool->q_capacity){
		while(temp!= NULL){
	    arrnumP1[i] = temp->pageHandle->pageNum;
			temp = temp->next;
			i++;
	  }

		int idx;
		for (idx = i; idx < pool->q_capacity; idx++) {
			arrnumP1[idx] = NO_PAGE;
		}
	}
	else  {
		// if buffer pool is full, loop through queue to find all dirty frames.
		temp = pool->front;
		while (temp) {
			arrnumP1[temp->index] = temp->pageHandle->pageNum;
			temp = temp->next;
		}

	}
	free(temp);
	return arrnumP1;
}


bool *getDirtyFlags (BM_BufferPool *const bm)
{
  int index = 0;
  Buffer_Storage *bs = (Buffer_Storage*)bm->mgmtData;
  bool *dirtyFlags = (bool *)malloc(sizeof(bool)*(bm->numPages));
	// bool dirtyFlags[bm->numPages];
	Queue *pool = bs->pool;
  Page_Frame *temp = pool->front;
	Page_Frame **mapping = bs->mapping;

	int i=0;
	if (pool->count < pool->q_capacity){
		while(temp!= NULL){
	    dirtyFlags[temp->index] = temp->is_dirty;
			temp = temp->next;
			i++;
	  }
		int idx;
		for (idx = i; idx < pool->q_capacity; idx++) {
			dirtyFlags[idx] = false;
		}
	}
	else  {
		// loop through queue.
		temp = pool->front;
		while(temp!= NULL){
	    dirtyFlags[temp->index] = temp->is_dirty;
			temp = temp->next;
			i++;
	  }
	}

	return dirtyFlags;
}

int *getFixCounts (BM_BufferPool *const bm)
{
  int index = 0;
  Buffer_Storage *bs = (Buffer_Storage*)bm->mgmtData;
  int *fixCount = (int *)malloc(sizeof(int)*bm->numPages);
	Queue *pool = bs->pool;
  Page_Frame *temp = pool->front;

	int i=0;
	if (pool->count < pool->q_capacity){
		while(temp!= NULL){
	    fixCount[i] = temp->fix_count;
			temp = temp->next;
			i++;
	  }

		int idx;
		for (idx = i; idx < pool->q_capacity; idx++) {
			fixCount[idx] = 0;
		}
	}
	else  {
		temp = pool->front;
		while (temp) {
			fixCount[temp->index] = temp->fix_count;
			temp = temp->next;
		}
	}
	free(temp);
 return fixCount;
}

int getNumReadIO (BM_BufferPool *const bm)
{
  Buffer_Storage *bs = (Buffer_Storage*)bm->mgmtData;
  Queue *pool = bs -> pool;
  return pool->readIO;
}

int getNumWriteIO (BM_BufferPool *const bm)
{
  Buffer_Storage *bs = (Buffer_Storage*)bm->mgmtData;
  Queue *pool = bs -> pool;
  return pool->writeIO;
}
