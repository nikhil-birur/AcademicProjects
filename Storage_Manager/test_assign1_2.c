#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "storage_mgr.h"
#include "dberror.h"
#include "test_helper.h"

char *testName;

/* test output files */
#define TESTPF "test_pagefile.bin"

static void testMultiplePagesContent(void);


int main(int argc, char *argv[])
{
	testName = "";
	initStorageManager();
	testMultiplePagesContent();
	return 0;
}


void testMultiplePagesContent(void) {
  SM_FileHandle fh;
  SM_PageHandle ph;

	testName = "test multiple pages content";

	ph = (SM_PageHandle) malloc(PAGE_SIZE);

  TEST_CHECK(createPageFile (TESTPF));
  TEST_CHECK(openPageFile (TESTPF, &fh));
  printf("created and opened file\n");

	//get block position.
	int blockPos = getBlockPos(&fh);
	ASSERT_TRUE( blockPos == 0, "expected 0 if open the page file for the first time.");

	//Test appendEmptyBlock.
	TEST_CHECK(appendEmptyBlock(&fh))
	ASSERT_TRUE(fh.totalNumPages == 2, "expect 2 for total nubmer of pages after appending 1 page to page file");
	ASSERT_TRUE(fh.curPagePos == 1, "expect 1 if one page is added to the page file");

	
	//test write block when the numPage is greater than the total number of pages, expected 'RC_READ_NON_EXISTING_PAGE'.
	int i;
  for (i=0; i < PAGE_SIZE; i++)
    ph[i] = (i % 10) + '0';
	ASSERT_TRUE(writeBlock(3, &fh, ph) == RC_READ_NON_EXISTING_PAGE, "expect RC_READ_NON_EXISTING_PAGE if numPage is bigger than the total number of pages in the page fiele");

	// write to the second block (zero-based indexing), so the first page is empty, 
	// we use this condition to test readPreviousBlock.
	TEST_CHECK(writeBlock(1, &fh, ph));
	TEST_CHECK(readPreviousBlock(&fh, ph));
  for (i=0; i < PAGE_SIZE; i++)
    ASSERT_TRUE((ph[i] == 0), "expected zero byte after reading from the previous block");
  printf("read from the previous page\n");

	// test readCurrentBlock.
	TEST_CHECK(readCurrentBlock(&fh, ph));
  for (i=0; i < PAGE_SIZE; i++)
    ASSERT_TRUE((ph[i] == (i % 10) + '0'), "character in page read from disk is the one we expected.");
  printf("reading current block\n");



	// increase the capacity to 5. 
	TEST_CHECK(ensureCapacity(5, &fh));
	blockPos = getBlockPos(&fh);
	ASSERT_TRUE(fh.totalNumPages == 5, "expected 5 for total number of pages after increasing page capacity to 5");
	ASSERT_TRUE(blockPos == 4, "expected 4 for current position after increasing page capacity to 5");


	// test writing to current block.
  for (i=0; i < PAGE_SIZE; i++)
    ph[i] = (i % 10) + '0';
	TEST_CHECK(writeCurrentBlock(&fh, ph));
	printf("writing to current block\n");

	// test read from the last block.
	TEST_CHECK(readLastBlock(&fh, ph));
  for (i=0; i < PAGE_SIZE; i++)
    ASSERT_TRUE((ph[i] == (i % 10) + '0'), "character in page read from disk is the one we expected.");
  printf("reading last block\n");
		
	TEST_CHECK(destroyPageFile (TESTPF));  

	TEST_DONE();
}
