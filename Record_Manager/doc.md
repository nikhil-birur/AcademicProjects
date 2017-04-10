## Table Header Format

Schema_formaddt: numAttr&attrName1&length1&attr1Type&attrName2&length2&attr2Type&attrName3&length3&aattr3Type


numAttr

attrName1

attr1Type

length1

attrName2

attr2Type

length2

attrName3

attr3Type

length3



typedef struct Schema
{
  int numAttr;
  char **attrNames;
  DataType *dataTypes;
  int *typeLength;
  int *keyAttrs;
  int keySize;
} Schema;


Name&TableCapacity&recordsPerPage&pageCount&recordCount&LastAccessed&Schema_format

test_table&336663&337&0&0&2016-11-06 16:05:35&3&a&DT_INT&0&b&DT_STRING&4&c&DT_INT&0&


page header;

First 48 bytes reserved. 


1&0&0&337
