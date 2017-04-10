#include <stdlib.h>
#include "dberror.h"
#include "expr.h"
#include "record_mgr.h"
#include "tables.h"
#include "test_helper.h"


#define ASSERT_EQUALS_RECORDS(_l,_r, schema, message)			\
  do {									\
    Record *_lR = _l;                                                   \
    Record *_rR = _r;                                                   \
    ASSERT_TRUE(memcmp(_lR->data,_rR->data,getRecordSize(schema)) == 0, message); \
    int i;								\
    for(i = 0; i < schema->numAttr; i++)				\
      {									\
        Value *lVal, *rVal;                                             \
		char *lSer, *rSer; \
        getAttr(_lR, schema, i, &lVal);                                  \
        getAttr(_rR, schema, i, &rVal);                                  \
		lSer = serializeValue(lVal); \
		rSer = serializeValue(rVal); \
        ASSERT_EQUALS_STRING(lSer, rSer, "attr same");	\
		free(lVal); \
		free(rVal); \
		free(lSer); \
		free(rSer); \
      }									\
  } while(0)

#define ASSERT_EQUALS_RECORD_IN(_l,_r, rSize, schema, message)		\
  do {									\
    int i;								\
    boolean found = false;						\
    for(i = 0; i < rSize; i++)						\
      if (memcmp(_l->data,_r[i]->data,getRecordSize(schema)) == 0)	\
	found = true;							\
    ASSERT_TRUE(0, message);						\
  } while(0)

#define OP_TRUE(left, right, op, message)		\
  do {							\
    Value *result = (Value *) malloc(sizeof(Value));	\
    op(left, right, result);				\
    bool b = result->v.boolV;				\
    free(result);					\
    ASSERT_TRUE(b,message);				\
   } while (0)


// test methods
static void testCreateAndReloadTombstoneList (void);
static void testInsertIntoTombstoneList(void);
static void testPrimaryKeyCheck(void);

// struct for test records
typedef struct TestRecord {
  int a;
  char *b;
  int c;
} TestRecord;

// helper methods
Record *testRecord(Schema *schema, int a, char *b, int c);
Schema *testSchema (void);
Record *fromTestRecord (Schema *schema, TestRecord in);

char *testName;

int main(int argc, char const *argv[]) {
	testName = "";
	testCreateAndReloadTombstoneList();
	testInsertIntoTombstoneList();
	testPrimaryKeyCheck();
	return 0;
}

void testCreateAndReloadTombstoneList(void) {
	testName = "test creating and reloading tombstone list";
	RM_TableData *table = (RM_TableData *) malloc(sizeof(RM_TableData));
  TestRecord inserts[] = {
    {1, "aaaa", 3},
    {2, "bbbb", 2},
    {3, "cccc", 1},
    {4, "dddd", 3},
    {5, "eeee", 5},
    {6, "ffff", 1},
    {7, "gggg", 3},
    {8, "hhhh", 3},
    {9, "iiii", 2},
    {10, "jjjj", 5},
  };
	int deletes[] = {
		9,
		6,
		7,
		8,
		5
	};
	int numInserts = 10, numDeletes=5, i;
	Record *r;
	RID *rids;
	Schema *schema;
	schema = testSchema();
	rids = (RID *) malloc(sizeof(RID) * numInserts);

	TEST_CHECK(initRecordManager(NULL));
	TEST_CHECK(createTable("test_table_d",schema));
	TEST_CHECK(openTable(table, "test_table_d"));

	// insert rows into table
	for(i = 0; i < numInserts; i++)
		{
			r = fromTestRecord(schema, inserts[i]);
			TEST_CHECK(insertRecord(table,r));
			rids[i] = r->id;
		}

	// delete rows from table.
	for(i = 0; i < numDeletes; i++)
		{
			TEST_CHECK(deleteRecord(table,rids[deletes[i]]));
		}

  TEST_CHECK(closeTable(table));

	char s[] = "(1,9)&(1,6)&(1,7)&(1,8)&(1,5)&";
	List *testList = deserializeTombstoneList(s);

  TEST_CHECK(openTable(table, "test_table_d"));

	Table_Header *tableheader = (Table_Header *)table->mgmtData;
	List *tmstone = tableheader->tombstone;

	ListNode *node1, *node2;

	node1 = testList->head;
	node2 = tmstone->head;

	ASSERT_EQUALS_INT(testList->itemCount, tmstone->itemCount, "same list size");

	while(node1 != NULL) {
		RID *id1 = (RID *)node1->value;
		RID *id2 = (RID *)node2->value;

		ASSERT_EQUALS_INT(id1->page, id2->page, "same page");
		ASSERT_EQUALS_INT(id1->slot, id2->slot, "same slot");

		node1 = node1->next;
		node2 = node2->next;
	}

	TEST_CHECK(closeTable(table));
	TEST_CHECK(deleteTable("test_table_r"));
	TEST_CHECK(shutdownRecordManager());

	free(table);
	free(rids);
	freeRecord(r);
	TEST_DONE();
}


void testInsertIntoTombstoneList(void) {
	testName = "test insert into tombstone list";
	RM_TableData *table = (RM_TableData *) malloc(sizeof(RM_TableData));
	TestRecord inserts[] = {
		{1, "aaaa", 3},
		{2, "bbbb", 2},
		{3, "cccc", 1},
		{4, "dddd", 3},
		{5, "eeee", 5},
		{6, "ffff", 1},
		{7, "gggg", 3},
		{8, "hhhh", 3},
		{9, "iiii", 2},
		{10, "jjjj", 5},
	};
	int deletes[] = {
		9,
		6,
		7,
		8,
		5
	};

	TestRecord newRecords[] = {
		{8, "xxxx", 8}, // slot is 7.
		{9, "zzzz", 9}, // slot is 8.
		{6, "yyyy", 6}, // slot is 5.
	};

	int newRids[] = {
		5, // {8, "xxxx", 8}
		8, // {9, "zzzz", 9}
		7  // {6, "yyyy", 6}
	};

	TestRecord finalR[] = {
		{1, "aaaa", 3},
		{2, "bbbb", 2},
		{3, "cccc", 1},
		{4, "dddd", 3},
		{5, "eeee", 5},
		{8, "xxxx", 8},  // replace slot 5 with {8, "xxxx", 8}
		{7, "gggg", 3},  // marked 'tombstone' but is not deleted.
		{6, "yyyy", 6},  // replace slot 7 with {6, "yyyy", 6}
		{9, "zzzz", 9},  // replace slot 8 with {9, "zzzz", 9}
		{10, "jjjj", 5}, // marked 'tombstone' but is not deleted.
	};


	int numInserts = 10, numDeletes=5, numNewInserts=3, i;
	Record *r;
	RID *rids;
	Schema *schema;
	testName = "test deleting records and inserting new records into those deleted slots";
	schema = testSchema();
	rids = (RID *) malloc(sizeof(RID) * numInserts);

	Config *con = (Config *)malloc(sizeof(Config));
	con->primaryKeyCheck = true;

	TEST_CHECK(initRecordManager(con));
	TEST_CHECK(createTable("test_table_d",schema));
	TEST_CHECK(openTable(table, "test_table_d"));

	// insert rows into table
	for(i = 0; i < numInserts; i++)
		{
			r = fromTestRecord(schema, inserts[i]);
			TEST_CHECK(insertRecord(table,r));
			rids[i] = r->id;
		}

	// delete rows from table.
	for(i = 0; i < numDeletes; i++)
		{
			TEST_CHECK(deleteRecord(table,rids[deletes[i]]));
		}

	TEST_CHECK(closeTable(table));


	TEST_CHECK(openTable(table, "test_table_d"));

	for(i = 0; i <numNewInserts; i++)
		{
			r = fromTestRecord(schema, newRecords[i]);
			TEST_CHECK(insertRecord(table,r));
			rids[newRids[i]] = r->id;
		}

	for(i = 0; i <numInserts; i++) {
		RID *rid = (RID *)malloc(sizeof(RID));
		rid->page = 1;
		rid->slot = i;
		if (i == 6 || i == 9) {
			ASSERT_EQUALS_INT(getRecord(table, *rid, r), RC_TUPLE_NOT_FOUND, "record has been deleted");
		}
		else {
			TEST_CHECK(getRecord(table, *rid, r));
			ASSERT_EQUALS_RECORDS(fromTestRecord(schema, finalR[i]), r, schema, "compare records");
		}
	}

  TEST_CHECK(closeTable(table))
	TEST_CHECK(deleteTable("test_table_r"));
	TEST_CHECK(shutdownRecordManager());

	free(table);
	free(rids);
	freeRecord(r);
	TEST_DONE();
}

void testPrimaryKeyCheck(void) {
	{
	  RM_TableData *table = (RM_TableData *) malloc(sizeof(RM_TableData));
	  TestRecord inserts[] = {
	    {1, "aaaa", 3},
	    {2, "bbbb", 2},
	    {3, "cccc", 1},
	    {4, "dddd", 3},
	    {5, "eeee", 5},
	    {6, "ffff", 1},
	    {7, "gggg", 3},
	    {8, "hhhh", 3},
	    {9, "iiii", 2}
	  };
	  int numInserts = 9, numInsertsTotal = 11, i;
	  Record *r;
	  RID *rids;
	  Schema *schema;
	  testName = "test inserting primary key duplicated records";
	  schema = testSchema();
	  rids = (RID *) malloc(sizeof(RID) * numInserts);

    Config *con = (Config *)malloc(sizeof(Config));

    con->primaryKeyCheck = true;

	  TEST_CHECK(initRecordManager(con));
	  TEST_CHECK(createTable("test_table_r",schema));
	  TEST_CHECK(openTable(table, "test_table_r"));


	  // insert rows into table
	  for(i = 0; i < numInserts; i++)
	    {
	      r = fromTestRecord(schema, inserts[i]);
	      TEST_CHECK(insertRecord(table,r));
	      rids[i] = r->id;
	    }

	  TEST_CHECK(closeTable(table));

	  TEST_CHECK(openTable(table, "test_table_r"));
	  TestRecord dupRecord[] = {
	    {2, "aaaa", 3},
		};

		r = fromTestRecord(schema, dupRecord[0]);
		ASSERT_EQUALS_INT(insertRecord(table, r), RC_DUPLICATED_PRIMARYKEY, "duplicated record insertion is detected.");


	  TestRecord newRecords[] = {
	    {10, "aaaa", 3},
	    {11, "aaaa", 3},
		};

		TestRecord finalR[] = {
	    {1, "aaaa", 3},
	    {2, "bbbb", 2},
	    {3, "cccc", 1},
	    {4, "dddd", 3},
	    {5, "eeee", 5},
	    {6, "ffff", 1},
	    {7, "gggg", 3},
	    {8, "hhhh", 3},
	    {9, "iiii", 2},
	    {10, "aaaa", 3},
	    {11, "aaaa", 3},
		}; // finalR doesn't include {2, "aaaa", 3} because of duplicated primary key check;

		for (i = 0; i < 2; i++) {
			r = fromTestRecord(schema, newRecords[i]);
			TEST_CHECK(insertRecord(table, r));
		}

		for(i = 0; i <numInsertsTotal; i++) {
			RID *rid = (RID *)malloc(sizeof(RID));
			rid->page = 1;
			rid->slot = i;
			TEST_CHECK(getRecord(table, *rid, r));
			ASSERT_EQUALS_RECORDS(fromTestRecord(schema, finalR[i]), r, schema, "compare records");
		}


	  TEST_CHECK(deleteTable("test_table_r"));
	  TEST_CHECK(deleteTable("test_table_r"));
	  TEST_CHECK(shutdownRecordManager());


    free(con);
	  free(rids);
	  free(table);
	  freeRecord(r);
	  TEST_DONE();
	}
}

Schema *
testSchema (void)
{
  Schema *result;
  char *names[] = { "a", "b", "c" };
  DataType dt[] = { DT_INT, DT_STRING, DT_INT };
  int sizes[] = { 0, 4, 0 };
  int keys[] = {0};
  int i;
  char **cpNames = (char **) malloc(sizeof(char*) * 3);
  DataType *cpDt = (DataType *) malloc(sizeof(DataType) * 3);
  int *cpSizes = (int *) malloc(sizeof(int) * 3);
  int *cpKeys = (int *) malloc(sizeof(int));

  for(i = 0; i < 3; i++)
    {
      cpNames[i] = (char *) malloc(2);
      strcpy(cpNames[i], names[i]);
    }
  memcpy(cpDt, dt, sizeof(DataType) * 3);
  memcpy(cpSizes, sizes, sizeof(int) * 3);
  memcpy(cpKeys, keys, sizeof(int));

  result = createSchema(3, cpNames, cpDt, cpSizes, 1, cpKeys);

  return result;
}

Record *
fromTestRecord (Schema *schema, TestRecord in)
{
  return testRecord(schema, in.a, in.b, in.c);
}

Record *
testRecord(Schema *schema, int a, char *b, int c)
{
  Record *result;
  Value *value;

  TEST_CHECK(createRecord(&result, schema));

  MAKE_VALUE(value, DT_INT, a);
  TEST_CHECK(setAttr(result, schema, 0, value));
  freeVal(value);

  MAKE_STRING_VALUE(value, b);
  TEST_CHECK(setAttr(result, schema, 1, value));
  freeVal(value);

  MAKE_VALUE(value, DT_INT, c);
  TEST_CHECK(setAttr(result, schema, 2, value));
  freeVal(value);

  return result;
}
