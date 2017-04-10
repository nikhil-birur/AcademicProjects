Buffer Manager :
-----------------

Implement a buffer manager based on assignment 1



How to run Buffer Manager (Test Case):
------------------------------------------

1) Navigate to the terminal where the Buffer Manager root folder is stored.

2) Compile : make

3) Run: ./bufferManager


Description of the Methods used and their implementation:
---------------------------------------------------------

Buffer Manager Functions

1) initBufferPool()

  a) Assign values(page filename, buffer size and strategy).
  b) create a queue based buffer pool storage space. 


2) shutdownBufferPool()

  a) Write dirty page frame to disk
  b) free space
  
  
3) forceFlushPool()

  a) Loop through queue to find all dirty pages and write them to disk.

4) markDirty ()

  a) Mark given page frame as dirty.
  
4) unpinPage ()

  a) Unpin a given page frame.
  
5) forcePage ()

  a) Write page frame to disk.
  
6) pinPage ()

  a) Pin a page based on strategy(FIFO and LRU)


Statistics Functions

1) getFrameContents()
  a) return a array of pageNum in each buffer pool
  
2) getDirtyFlags()
  a) return a array of dirty flag in each buffer pool
  
3) getFixCounts ()
  a) return a array of fix count in each buffer pool
  
4) getNumReadIO ()
  a) return total number of reading from page file. 
  
5) getNumWriteIO ()
  a) return total number of writing from page file. 


Strategies 

1. FIFO
Our FIFO strategy makes use for Queue's 'First in first out' property, so each time a new page frame needs to be 
added to buffer pool, simply add it to the rear of the queue, and remove the front of the queue if a page frame 
is being removed from buffer pool. Each operation takes O(1), which is very efficient.


2. LRU

Our LRU is also based on queue, it means the least recently used page frame can be pushed to the front of the queue.
If there is a repeated page fame, we first remove that page frame, and then add it to the rear of the queue, 
Each operation still take O(1).



