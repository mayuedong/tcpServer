#ifndef _LIST_H_
#define _LIST_H_

#include "common.h"

#define _START_HEAD_	0
#define _START_TAIL_	1

typedef struct listNode
{
	struct listNode *prev;
	struct listNode *next;
	void *pValue;
}listNode;

typedef struct list
{
	listNode *pHead;
	listNode *pTail;
	int len;
	void* (*dup)(void *ptr);
	void (*release)(void *ptr);
	int (*match)(void *ptr, void *pValue);
}list;

typedef struct listIter
{
	listNode *pNode;
	int iDirection;
}listIter;

int listLength(list *pList);
listNode *listFirst(list *pList);
listNode *listLast(list *pList);
listNode *listPrevNode(listNode *pNode);
listNode *listNextNode(listNode *pNode);
void *listNodeValue(listNode *pNode);

list *listCreate(void);
void listClear(list *pList);
void listRelease(list *pList);
list *listAddHead(list *pList,  void *pValue);
list *listAddTail(list *pList,  void *pValue);
list *listAdd(list *pList,  void *pValue, int flag);
list *listInsertFront(list *pList,  listNode *pOldNode,  void *pValue);
list *listInsertAfter(list *pList,  listNode *pOldNode,  void *pValue);
//list *InsertNode(list *pList,  listNode *pOldNode,  void *pValue, int flag);
void listDelete(list *pList,  listNode *pNode);

listIter *listGetIterHead(list *pList);
listIter *listGetIterTail(list *pList);
//listIter *GetListIter(list *pList, int flag);
listIter *SetIterFront(list *pList, listIter *pIt);
listIter *SetIterBack(list *pList, listIter *pIt);
//listIter *SetIter(list *pList, listIter *pIt, int flag);
void listReleaseIter(listIter *pIt);
listNode *listGetNext(listIter *pIt);

list *listCopy(list *pOldList);
listNode *listFind(list *pList, void *pValue);
listNode *listGetNode(list *pList, int index);
list * listRotate(list *pList);


#endif//_LIST_H_
