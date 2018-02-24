#ifndef _DICT_H_
#define _DICT_H_

#include "common.h"

typedef struct dictEntry
{
	void *key;
	void *val;
	struct dictEntry *next;
}dictEntry;

typedef struct dictht
{
	dictEntry **table;
	int size;
	int used;
	int index;
}dictht;

typedef struct dictType
{
	void *(*dupKey)(void *privdata, void *key);
	void *(*dupVal)(void *privdata, void *val);
	void (*ReleaseKey)(void *privdata, void *key);
	void (*ReleaseVal)(void *privdata, void *val);
	int (*compareKey)(void *privdata, void *key1, void *key2);
	unsigned int (*hashFunction)(void *key);
}dictType;

typedef struct dict
{
	dictht ht[2];
	dictType *type;
	void *privdata;
	int rehashIndex;
	int iterator;
}dict;

typedef struct dictIterator
{
	dict *d;
	int table;
	int index;
	dictEntry *entry;
	dictEntry *nextEntry;
}dictIterator;

#define DICT_TABLE_INITIAL_SIZE	4
#define DICT_TABLE_MAX_SIZE	2147483647 

void dictReleaseKey(dict *d, dictEntry *entry);
void dictReleaseVal(dict *d, dictEntry *entry);
void dictSetKey(dict *d, dictEntry *entry, void *key);
void dictSetVal(dict *d, dictEntry *entry, void *val);
int dictCompareKey(dict *d, void *key1, void *key2);
size_t dictHashKey(dict *d, void *key);

dict *dictCreate(dictType *type, void *privdata);
int dictExpand(dict *d, int size);
int dictResize(dict *d);
int dictAdd(dict *d, void *key, void *val);
int dictDelete(dict *d, void *key);


int dictRehash(dict *d, int n);
int dictRehashTime(dict *d, int ms);

dictEntry *dictFind(dict *d, void *key);
void *dictGetVal(dictEntry *entry);

dictEntry *dictRand(dict *d);
int dictReplace(dict *d, void *key, void *val);

void dictClear(dict *d);
void dictRelease(dict *d);

void dictEnableResize(void);
void dictDisableResize(void);

dictIterator *dictGetIterator(dict *d);
void dictReleaseIterator(dictIterator *iter);
dictEntry *dictNext(dictIterator *it);


#endif//_DICT_H_
