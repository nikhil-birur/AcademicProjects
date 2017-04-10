Record Manager :
-----------------

The goal of this assignment is to implement a simple record manager that allows navigation through records, and inserting and deleting records. The record manager handles tables with a fixed schema. Clients can insert records, delete records, update records, and scan through the records in a table. A scan is associated with a search condition and only returns records that match the search condition. Each table should be stored in a separate page file and your record manager should access the pages of the file through the buffer manager implemented in the last assignment.


Additional Return Code :
------------------------

1. RC_DUPLICATED_PRIMARYKEY 402
2. RC_NOT_FOUND_IN_TOMBSTONE 403
3. RC_TUPLE_NOT_FOUND 404


Additional Test Cases:

1. testCreateAndReloadTombstoneList()

test creating a tombstone list for stored the records that have been deleted and reload the list after open the table file again.

2. testInsertIntoTombstoneList()

test inserting into a tombstone list.

3. testPrimaryKeyCheck()

test checking primary key constraints.



Description of the Methods used and their implementation:
---------------------------------------------------------

1) initRecordManager Function:
 	This function initializes the record manager.

	Return Value : RC_OK

********************************************************************************************

2) shutdownRecordManager Function:
	This function shuts down the record manager.

	Return Value : RC_OK

********************************************************************************************

 3) createTable Function:
	create the underlying page file and store information about the schema, free-space and so on in the Table Information pages.

	Return Value : RC_OK

********************************************************************************************

4) openTable Function:
 	Opens the Table before insert, delete, update operation are performed.

	Return Value : RC_OK

********************************************************************************************

5) closeTable Funtion:
	Writes the changes to table and closes the file.

	Return Value : RC_OK

********************************************************************************************

6) deleteTable Function:
	Deletes the table page file.

	Return Value : RC_OK

********************************************************************************************

7) getNumTuples Function:
 	Returns the number of Tuples in a table.

	Return Value : RC_OK

********************************************************************************************

 8) insertRecord Function:
	Inserts a new record with an unique RID in the a particular slot of a page.

	Return Value : RC_OK

********************************************************************************************

9) deleteRecord Function:
 	Deletes a record from the table.

	Return Value : RC_OK

********************************************************************************************

10) updateRecord Function:
	Updates a record in the table.

	Return Value : RC_OK

********************************************************************************************

 11) getRecord Function:
 	Gets(returns) a record from the table with a particular RID.

	Return Value : RC_OK

********************************************************************************************

 12) startscan Function:
	Starting a scan initializes the RM_ScanHandle data structure passed as an argument to startScan. Afterwards, calls to the next method should return the next tuple that fulfills the scan condition. If NULL is passed as a scan condition, then all tuples of the table should be returned. next should return RC_RM_NO_MORE_TUPLES once the scan is completed and RC_OK otherwise (unless an error occurs of course).

	Return Value : RC_OK

********************************************************************************************

 13) next Function:
	Returns the next record based on the given condition.

	Return Value : RC_OK

********************************************************************************************

 14) closeScan Function:
	Closes the scan operations.

	Return Value : RC_OK

********************************************************************************************

 15) getRecordSize Function:
	Returns the Size of the records.

	Return Value : int

********************************************************************************************

 16) createSchema Function:
	Creates a new Schema.

	Return Value : Schema

********************************************************************************************

 17) freeSchema Function:
 	Frees the Schema.

	Return Value : RC_OK

********************************************************************************************

 18) createRecord Function:
 	Creates a new record.

	Return Value : RC_OK

********************************************************************************************

 19) freeRecord Function:
 	Free the memory space occupied by a record and return the status.

	Return Value : RC_OK

********************************************************************************************

 20) getAttr Function:
 	Returns the attribute value.

	Return Value : RC_OK

********************************************************************************************

 21) setAttr Function:
 	Sets the attribute value.

	Return Value : RC_OK

/*******************************************************************************************
*


Additional helper functions:
----------------------------

********************************************************************************************
*
* 1) Deserialization functions
*
*   deserializeRecord(Schema *schema, char *recordString, RID id)
*   deserializePageHeader(char *str, Page_Header *pageHeader)
*   deserializeTombstoneNode(char *str)
*   deserializeTombstoneList(char *str)
*
********************************************************************************************
*
* 2) Serialization functions:
*   serializeTombstonList(List *l)
*   generateTableInfo(RM_TableData *rel)
*   generatePageHeader(RM_TableData *rel, Page_Header *pageHeader)
*
********************************************************************************************
*
* 3) Check primiary key constraints
*   primaryKeyCheck(RM_TableData *rel, Record *r)
*   find(List *l, RID id)
*
********************************************************************************************
*
* 4) List operations:
*   createList(void);
*   insert(List *l, void *item)
*   popTail(List *l)
*   releaseList(List *l)
*   printList(List *l)
*
/*******************************************************************************************

How to run Record Manager (Test Case):
------------------------------------------

1) Navigate to the terminal where the Record Manager root folder is stored.

2) Compile : make -f makefile

3) Run: ./recordManager
********************************************************************************************

How to run Record Manager (Extra Test Case):
------------------------------------------

1) Navigate to the terminal where the Record Manager root folder is stored.

2) Compile : make -f makefile1

3) Run: ./recordManager
