#ifndef RECORD_MGR_H
#define RECORD_MGR_H

#include "dberror.h"
#include "expr.h"
#include "tables.h"
#include "list.h"
// #include "table_mgr.h"

// Bookkeeping for scans
typedef struct RM_ScanHandle
{
  RM_TableData *rel;
  void *mgmtData;
} RM_ScanHandle;


typedef struct ScanInfo {
  Expr *cond;
	RID curRID;
} ScanInfo;


// table and manager
extern RC initRecordManager (void *mgmtData);
extern RC shutdownRecordManager ();
extern RC createTable (char *name, Schema *schema);
extern RC openTable (RM_TableData *rel, char *name);
extern RC closeTable (RM_TableData *rel);
extern RC deleteTable (char *name);
extern int getNumTuples (RM_TableData *rel);

// handling records in a table
extern RC insertRecord (RM_TableData *rel, Record *record);
extern RC deleteRecord (RM_TableData *rel, RID id);
extern RC updateRecord (RM_TableData *rel, Record *record);
extern RC getRecord (RM_TableData *rel, RID id, Record *record);

// scans
extern RC startScan (RM_TableData *rel, RM_ScanHandle *scan, Expr *cond);
extern RC next (RM_ScanHandle *scan, Record *record);
extern RC closeScan (RM_ScanHandle *scan);

// dealing with schemas
extern int getRecordSize (Schema *schema);
extern Schema *createSchema (int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys);
extern RC freeSchema (Schema *schema);

// dealing with records and attribute values
extern RC createRecord (Record **record, Schema *schema);
extern RC freeRecord (Record *record);
extern RC getAttr (Record *record, Schema *schema, int attrNum, Value **value);
extern RC setAttr (Record *record, Schema *schema, int attrNum, Value *value);


// extra table and header related functions.
RC initTableManager(Table_Header *manager, Schema *schema);
char *generateTableInfo(RM_TableData *rel);
char *generatePageHeader(RM_TableData *rel, Page_Header *pageHeader);
int currentTime(char *buffer);
int tableInfoLength(RM_TableData *rel);
int tableLength(RM_TableData *rel);
int schemaLength(Schema *schema);
RC parseTableHeader(RM_TableData *rel, char *stringHeader);
DataType stringToDatatype(char *token);
RC initPageHeader(RM_TableData *rel, Page_Header *pageHeader, int pageId);
Record *deserializeRecord(Schema *schema, char *recordString, RID id);
RC deserializePageHeader(char *str, Page_Header *pageHeader);
RID *deserializeTombstoneNode(char *str);
List *deserializeTombstoneList(char *str);
char *serializeTombstonList(List *l);
RC primaryKeyCheck(RM_TableData *rel, Record *r);
RC find(List *l, RID id);

#endif // RECORD_MGR_H
