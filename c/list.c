#include "memory.h"
#include "list.h"


int listLength(list *pList)
{
	if (NULL == pList)
		return 0;
	return pList->len;
}
listNode *listFirst(list *pList)
{
	if (NULL == pList)
		return NULL;
	return pList->pHead;
}
listNode *listLast(list *pList)
{
	if (NULL == pList)
		return NULL;
	return pList->pTail;
}
listNode *listPrevNode(listNode *pNode)
{
	if (NULL == pNode)
		return NULL;
	return pNode->prev;
}
listNode *listNextNode(listNode *pNode)
{
	if (NULL == pNode)
		return NULL;
	return pNode->next;
}
void *listNodeValue(listNode *pNode)
{
	if (NULL == pNode)
		return NULL;
	return pNode->pValue;
}

list *listCreate(void)
{
	list *pList = (list*)reMalloc(sizeof(list));
	return pList;
}

void listClear(list *pList)
{
	if (NULL == pList)
	{
		return;
	}
	listNode *pNode = pList->pHead;
	while (NULL != pNode)
	{
		if (NULL != pList->release)
		{
			pList->release(pNode->pValue);
		}
		pList->pHead = pNode->next;
		reFree(pNode);
		pNode = pList->pHead;
	}
	pList->pHead = NULL;
	pList->pTail = NULL;
	pList->len = 0;
}

void listRelease(list *pList)
{
	listClear(pList);
	reFree(pList);
}

list *listAddHead(list *pList, void *pValue)
{
	if (NULL == pList || NULL == pValue)
	{
		return NULL;
	}
	listNode *pNode = (listNode*)reMalloc(sizeof(listNode));
	
	pNode->pValue = pValue;
	if (NULL == pList->pHead)
	{
		pList->pHead = pNode;
		pList->pTail = pNode;
	}
	else
	{
		pNode->next = pList->pHead;
		pNode->prev = NULL;
		pList->pHead->prev = pNode;
		pList->pHead = pNode;
	}
	pList->len++;
	return pList;
}

list *listAddTail(list *pList, void *pValue)
{
	if (NULL == pList || NULL == pValue)
	{
		return NULL;
	}
	listNode *pNode = (listNode*)reMalloc(sizeof(listNode));
	
	pNode->pValue = pValue;
	if (NULL == pList->pTail)
	{
		pList->pHead = pNode;
		pList->pTail = pNode;
	}
	else
	{
		pNode->next = NULL;
		pNode->prev = pList->pTail;
		pList->pTail->next = pNode;
		pList->pTail= pNode;
	}
	pList->len++;
	return pList;
}

list *listInsertFront(list *pList, listNode *pOldNode, void *pValue)
{
	if (NULL == pList || NULL == pOldNode || NULL == pValue)
	{
		return NULL;
	}
	listNode *pNode = (listNode *)reMalloc(sizeof(listNode));
	
	pNode->pValue = pValue;
	pNode->next = pOldNode;
	pNode->prev = pOldNode->prev;
	pOldNode->prev = pNode;
	if (pList->pHead == pOldNode)
	{
		pList->pHead = pNode;	
	}
	else
	{
		pNode->prev->next = pNode;
	}
	pList->len++;
	return pList;
}

list *listInsertAfter(list *pList, listNode *pOldNode, void *pValue)
{
	if (NULL == pList || NULL == pOldNode || NULL == pValue)
	{
		return NULL;
	}
	listNode *pNode = (listNode *)reMalloc(sizeof(listNode));

	pNode->pValue = pValue;
	pNode->prev = pOldNode;
	pNode->next = pOldNode->next;
	pOldNode->next = pNode;
	if (pList->pTail == pOldNode)
	{
		pList->pTail = pNode;
	}
	else
	{
		pNode->next->prev = pNode;
	}
	pList->len++;
	return pList;
}

void listDelete(list *pList, listNode *pNode)
{
	if (NULL == pList || NULL == pNode)
	{
		return;
	}
	
	if (pList->pHead == pNode)
	{
		pList->pHead = pNode->next;
	}
	else
	{
		pNode->prev->next = pNode->next;
	}
	
	if (pList->pTail == pNode)
	{
		pList->pTail = pNode->prev;
	}
	else
	{
		pNode->next->prev = pNode->prev;
	}
	
	if (NULL != pList->release)
	{
		pList->release(pNode->pValue);
	}
	reFree(pNode);
	pList->len--;
}

listIter *listGetIterHead(list *pList)
{
	if (NULL == pList || NULL == pList->pHead)
	{
		return NULL;
	}
	listIter *pIt = (listIter *)reMalloc(sizeof(listIter));
	
	pIt->pNode = pList->pHead;
	pIt->iDirection = _START_HEAD_;
	return pIt;
}

listIter *listGetIterTail(list *pList)
{
	if (NULL == pList || NULL == pList->pTail)
	{
		return NULL;
	}
	listIter *pIt = (listIter *)reMalloc(sizeof(listIter));
	
	pIt->pNode = pList->pTail;
	pIt->iDirection = _START_TAIL_;
	return pIt;
}

listIter *SetIterFront(list *pList, listIter *pIt)
{
	if (NULL == pList || NULL == pList->pHead || NULL == pIt)
	{
		return NULL;
	}
	pIt->pNode = pList->pHead;
	pIt->iDirection = _START_HEAD_;
	return pIt;
}

listIter *SetIterBack(list *pList, listIter *pIt)
{
	if (NULL == pList || NULL == pList->pTail || NULL == pIt)
	{
		return NULL;
	}
	pIt->pNode = pList->pTail;
	pIt->iDirection = _START_TAIL_;
	return pIt;
}
	
void listReleaseIter(listIter *pIt)
{
	reFree(pIt);
}

listNode *listGetNext(listIter *pIt)
{
	if (NULL == pIt || NULL == pIt->pNode)
	{
		return NULL;
	}
	if (_START_HEAD_ == pIt->iDirection)
	{
		pIt->pNode = pIt->pNode->next;
	}
	else
	{
		pIt->pNode = pIt->pNode->prev;
	}
	return pIt->pNode;
}

list *listCopy(list *pOldList)
{
	if (NULL == pOldList || NULL == pOldList->pHead)
	{
		return NULL;
	}
	list *pNewList = listCreate();
	
	pNewList->match = pOldList->match;
	pNewList->release = pOldList->release;
	pNewList->dup = pOldList->dup;

	listNode *pOldNode = pOldList->pHead;
	void *pValue = NULL;
	while (NULL != pOldNode)
	{
		if (NULL != pNewList->dup)
		{
			pValue = pNewList->dup(pOldNode->pValue);
		}
		else
		{
			pValue = pOldNode->pValue;
		}
		if (NULL == listAddTail(pNewList, pValue) )
		{
			listRelease(pNewList);
			return NULL;
		}
		pOldNode = pOldNode->next;
	}
	return pNewList;
}

listNode *listFind(list *pList, void *pValue)
{
	if (NULL == pList || NULL == pList->pHead || NULL == pValue)
	{
		return NULL;
	}

	listNode *pNode = pList->pHead;
	while (NULL != pNode)
	{
		if (NULL != pList->match)
		{
			if (0 == pList->match(pNode, pValue))
			{				
				return pNode;
			}
		}
		else
		{
			if (pNode->pValue == pValue)
			{
				return pNode;
			}
		}
		pNode = pNode->next;
	}
	return NULL;
}

listNode *listGetNode(list *pList, int index)
{
	if (NULL == pList || NULL == pList->pHead)
	{
		return NULL;
	}
	listNode *pNode = NULL;
	if (0 <= index)
	{
		pNode = pList->pHead;
		while (index-- && NULL != pNode)
		{
			pNode = pNode->next;
		}
	}
	else
	{
		index = (-index) - 1;
		pNode = pList->pTail;
		while (index-- && NULL != pNode)
		{
			pNode = pNode->prev;
		}
	}
	return pNode;
}

list * listRotate(list *pList)
{
	if (NULL == pList || 1 >= pList->len || NULL == pList->pHead || NULL == pList->pTail)
	{
		return NULL;
	}
	pList->pHead->prev = pList->pTail;
	pList->pTail->next = pList->pHead;

	pList->pHead = pList->pTail;
	pList->pTail = pList->pTail->prev;

	pList->pHead->prev = NULL;
	pList->pTail->next = NULL;
	return pList;
}

#ifdef _DEBUG_

void Display(list *pList)
{
	if (NULL == pList)
	{
		return;
	}
	listNode *pNode = pList->pHead;
	while (NULL != pNode)
	{
		printf("%d", *(int *)(pNode->pValue));
		pNode = pNode->next;
	}
	printf("\r\n");
}

int match(void *ptr, void *pValue)
{
	if (NULL == ptr || NULL == pValue)
	{
		return -1;
	}
	
	listNode *pNode = (listNode *)ptr;
	if (*(int *)(pNode->pValue) == *(int *)pValue)
	{
		return 0;
	}
	 return -1;
}
int main(void)
{
	list *pList = listCreate();
	pList->match = match;
	if (NULL == pList)
	{
		return -1;
	}
	
	int iData1 = 1;
	if (NULL == listAddTail(pList,  &iData1))
	{
		return -1;
	}
	
	int iData2 = 2;
	if (NULL == listAddHead(pList,  &iData2))
	{
		return -1;
	}
	//21
	int iData3 = 3;
	if (NULL == listAdd(pList, &iData3, _START_HEAD_))
	{
		return -1;
	}

	int iData4 = 4;
	if (NULL == listAdd(pList, &iData4, _START_TAIL_))
	{
		return -1;
	}
	//3214
	
	int iData5 = 5;
	if (NULL == pList || NULL == listInsertFront(pList,  pList->pHead,  &iData5))
	{
		return -1;
	}
	//53214
	
	int iData6 = 6;
	if (NULL == pList || NULL == listInsertFront(pList,  pList->pTail,  &iData6))
	{
		return -1;
	}
	//532164
	
	int iData7 = 7;
	if (NULL == pList || NULL == listInsertAfter(pList,  pList->pHead,  &iData7))
	{
		return -1;
	}
	//5732164
	
	int iData8 = 8;
	if (NULL == pList || NULL == listInsertAfter(pList,  pList->pTail,  &iData8))
	{
		return -1;
	}
	//57321648
	Display(pList);
	printf("----------------------------------------------------------\r\n");
	
	list *pNewList = listCopy(pList);
	if (NULL == pNewList)
	{
		return -1;
	}
	
	listClear(pList);
	listRelease(pList);
	pList = NULL;
	
	if (NULL == listRotate(pNewList))
	{
		return -1;
	}
	//85732164
	Display(pNewList);
	printf("----------------------------------------------------------\r\n");
	
	int iFindData = 8;
	listDelete(pNewList, listFind(pNewList, &iFindData));
	//5732164

	
	iFindData = 4;
	listDelete(pNewList, listFind(pNewList, &iFindData));
	//573216
	
	iFindData = 1;
	listDelete(pNewList, listFind(pNewList, &iFindData));
	//57326
	Display(pNewList);
	printf("----------------------------------------------------------\r\n");
	
	listNode *pNode = NULL;
	int index = -5;
	while (5 > index)
	{
		if (NULL != (pNode = listGetNode(pNewList, index)))
		{
			printf("pValue[%d] = %d\r\n", index, *(int *)(pNode->pValue));
		}
		index++;
	}
	//57326 57326
	
	listIter *itHead = listGetIterHead(pNewList);
	if (NULL != itHead && NULL != itHead->pNode)
	{
		printf("%d\r\n", *(int *)(itHead->pNode->pValue));
	}
	//5
	listIter *itTail = listGetIterTail(pNewList);
	if (NULL != itHead && NULL != itTail->pNode)
	{
		printf("%d\r\n", *(int *)(itTail->pNode->pValue));
	}
	//6
	printf("----------------------------------------------------------\r\n");
	
	listIter itTemp = *itTail;
	if (NULL == SetIterFront(pNewList, &itTemp) || itTemp.pNode != itHead->pNode)
	{
		return -1;
	}
	else
	{
		printf("%d\r\n", *(int *)(itTemp.pNode->pValue));
	}
	//5

	itTemp = *itHead;
	if (NULL == SetIterBack(pNewList, &itTemp) || itTemp.pNode != itTail->pNode)
	{
		return -1;
	}
	else
	{
		printf("%d\r\n", *(int *)(itTemp.pNode->pValue));
	}
	//6
	printf("----------------------------------------------------------\r\n");
	while (NULL != (pNode = listGetNext(itHead)))
	{
		printf("%d", *(int *)(pNode->pValue));
	}
	printf("\r\n");
	//7326
	while (NULL != (pNode = listGetNext(itTail)))
	{
		printf("%d", *(int *)(pNode->pValue));
	}
	printf("\r\n");
	//2375
	listReleaseIter(itHead);
	listReleaseIter(itTail);
	listRelease(pNewList);
	pNewList = NULL;
	int sizeMemory = GetUsedMemory();
	printf("memory size = %d\r\n", sizeMemory);
	return 0;
}

#endif//_DEBUG_
