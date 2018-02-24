#include "memory.h"
#include "dict.h"

static int dict_resize = 1;
static int dict_ratio = 5;
static unsigned int dict_hash_function_seed = 5381;

void dictSetHashFunctionSeed(unsigned int seed) 
{
    dict_hash_function_seed = seed;
}

unsigned int dictGetHashFunctionSeed() 
{
    return dict_hash_function_seed;
}

unsigned int dictIntHashFunction(unsigned int key)
{
    key += ~(key << 15);
    key ^=  (key >> 10);
    key +=  (key << 3);
    key ^=  (key >> 6);
    key += ~(key << 11);
    key ^=  (key >> 16);
    return key;
}

unsigned int dictGenHashFunction(void *key, int len)
{
	unsigned int seed = dict_hash_function_seed;
	const int m = 0x5bd1e995;
	const int r = 24;

	unsigned int h = seed ^ len;
	const unsigned char *data = (const unsigned char *)key;

	while(len >= 4) 
	{
	    unsigned int k = *(unsigned int*)data;
	    k *= m;
	    k ^= k >> r;
	    k *= m;

	    h *= m;
	    h ^= k;

        data += 4;
        len -= 4;
	}
	switch(len) 
	{
		case 3: h ^= data[2] << 16;
		case 2: h ^= data[1] << 8;
		case 1: h ^= data[0]; h *= m;
	};

	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;

	return (unsigned int)h;
}

unsigned int dictGenCaseHashFunction(unsigned char *buf, int len) 
{
	unsigned int hash = dict_hash_function_seed;
	while (len--)
	{
		hash = ((hash << 5) + hash) + (tolower(*buf++));
	}
	return hash;
}

void dictReleaseKey(dict *d, dictEntry *entry)
{
	if (NULL == d || NULL == entry)
	{
		return;
	}
	if (NULL != d->type && NULL != d->type->ReleaseKey)
	{
		d->type->ReleaseKey(d->privdata, entry->key);
	}
	else
	{
		entry->key = NULL;
	}
}

void dictReleaseVal(dict *d, dictEntry *entry)
{
	if (NULL == d || NULL == entry)
	{
		return;
	}
	if (NULL != d->type && NULL != d->type->ReleaseVal)
	{
		d->type->ReleaseVal(d->privdata, entry->val);
	}
	else
	{
		entry->val = NULL;
	}
}

void dictSetKey(dict *d, dictEntry *entry, void *key)
{
	if (NULL == d || NULL == entry || NULL == key)
	{
		return;
	}
	if (NULL != d->type && NULL != d->type->dupKey)
	{
		entry->key = d->type->dupKey(d->privdata, key);
	}
	else
	{
		entry->key = key;
	}
}

void dictSetVal(dict *d, dictEntry *entry, void *val)
{
	if (NULL == d || NULL == entry || NULL == val)
	{
		return;
	}
	if (NULL != d->type && NULL != d->type->dupVal)
	{
		entry->val = d->type->dupVal(d->privdata, val);
	}
	else
	{
		entry->val = val;
	}
}

int dictCompareKey(dict *d, void *key1, void *key2)
{
	if (NULL != d && NULL != d->type && NULL != d->type->compareKey)
	{
		return d->type->compareKey(d->privdata, key1, key2);
	}
	return key1 == key2;
}

unsigned int dictHashKey(dict *d, void *key)
{	
	if (NULL == d || NULL == key)
	{
		return -1;
	}
	if (NULL != d->type && NULL != d->type->hashFunction)
	{
		return d->type->hashFunction(key);
	}
	return -1;
}

static void dictResetHt(dictht *ht)
{
	ht->size = 0;
	ht->index = 0;
	ht->used = 0;
	ht->table = NULL;
}

dict *dictCreate(dictType *type, void *privdata)
{
	dict *d = reMalloc(sizeof(dict));
	
	dictResetHt(d->ht);
	dictResetHt(d->ht + 1);
	
	d->iterator = 0;
	d->rehashIndex = -1;
	d->type = type;
	d->privdata = privdata;
	return d;
}

static int dictExpandSize(int size)
{
	int temp = DICT_TABLE_INITIAL_SIZE;
	if (size >= DICT_TABLE_MAX_SIZE)
	{
		return DICT_TABLE_MAX_SIZE;
	}
	while (temp < size)
	{
		temp *= 2;
	}
	return temp;
}

int dictExpand(dict *d, int size)
{
	if (NULL == d || -1 != d->rehashIndex || size < d->ht[0].used)
	{
		return ERROR;
	}
	
	size = dictExpandSize(size);
	dictEntry **table = reMalloc(sizeof(dictEntry *) * size);
	
	dictht ht;
	ht.table = table;
	ht.size = size;
	ht.index = size - 1;
	ht.used = 0;

	if (NULL == d->ht[0].table)
	{
		d->ht[0] = ht;
		return OK;
	}
	d->ht[1] = ht;
	d->rehashIndex = 0;
	return OK;
}

int dictResize(dict *d)
{
	if (NULL == d || -1 != d->rehashIndex || 1 != dict_resize)
	{
		return ERROR;
	}
	int size = (d->ht[0].used > DICT_TABLE_INITIAL_SIZE) ? (d->ht[0].used) : DICT_TABLE_INITIAL_SIZE;
	return dictExpand(d, size);
}

int dictRehash(dict *d, int n)
{
	if (NULL == d || 0 >= n || -1 == d->rehashIndex)
	{
		return ERROR;
	}
	if (NULL == d->ht[0].table || NULL == d->ht[1].table)
	{
		return ERROR;
	}

	while (n--)
	{
		if (0 == d->ht[0].used)
		{
			reFree(d->ht[0].table);
			d->ht[0] = d->ht[1];
			dictResetHt(&(d->ht[1]));
			d->rehashIndex = -1;
			return OK;
		}

		while (NULL == d->ht[0].table[d->rehashIndex])
		{
			d->rehashIndex++;
		}
		
		dictEntry *entry = d->ht[0].table[d->rehashIndex];
		while (NULL != entry)
		{
			dictEntry *nextEntry = entry->next;
			unsigned int index = dictHashKey(d, entry->key) & d->ht[1].index;
			entry->next = d->ht[1].table[index];
			d->ht[1].table[index] = entry;
			d->ht[0].used--;
			d->ht[1].used++;
			entry = nextEntry;
		}
		d->ht[0].table[d->rehashIndex] = NULL;
		d->rehashIndex++;
	}
	return ERROR;
}

int  dictRehashTime(dict *d, int ms)
{
	if (NULL == d || 0 >= ms)
	{
		return -1;
	}
	long long startTime = timeMs();
	int count = 0;
	while (OK == dictRehash(d, 100))
	{
		count += 100;
		if (timeMs() - startTime >= ms)
		{
			break;
		}
	}
	return count;
}

static void rehashStep(dict *d)
{
	if (0 == d->iterator)
	{
		dictRehash(d, 1);
	}
}

static int dictExpandIfNeed(dict *d)
{	
	if (-1 != d->rehashIndex)
	{
		return OK;
	}
	if (NULL == d->ht->table)
	{
		return dictExpand(d, DICT_TABLE_INITIAL_SIZE);
	}
	if (d->ht[0].used > d->ht[0].size && (1 == dict_resize || dict_ratio < d->ht[0].used / d->ht[0].size))
	{
		return dictExpand(d, d->ht[0].used * 2);
	}
	return OK;
}

static int KeyToIndex(dict *d, void *key)
{
	if (ERROR == dictExpandIfNeed(d))
	{
		return -1;
	}
	int index = (int)dictHashKey(d, key);
	int j = 0;
	int i = 0;
	for (i = 0; i < 2; i++)
	{
		j = index & d->ht[i].index;
		dictEntry *entry = d->ht[i].table[j];
		while (NULL != entry)
		{
			if (0 == dictCompareKey(d,entry->key, key))
			{
				return -1;
			}
			entry = entry->next;
		}
		if (-1 == d->rehashIndex)
		{
			break;
		}
	}
	return j;
}

int dictAdd(dict *d, void *key, void *val)
{
	if (NULL == d || NULL == key || NULL == val)
	{
		return ERROR;
	}
	if (-1 != d->rehashIndex)
	{
		rehashStep(d);
	}
	int index = KeyToIndex(d, key);
	if (-1 == index)
	{
		return ERROR;
	}
	
	dictEntry *entry = reMalloc(sizeof(dictEntry));
	dictSetKey(d, entry, key);
	dictSetVal(d, entry, val);
	
	dictht *ht = (-1 == d->rehashIndex) ? &(d->ht[0]) : &(d->ht[1]);
	
	entry->next = ht->table[index];
	ht->table[index] = entry;
	ht->used++;
	return OK;
}

int dictDelete(dict *d, void *key)
{
	if (NULL == d || NULL == key || NULL == d->ht || 0 == d->ht[0].used)
	{
		return ERROR;
	}
	
	if (-1 != d->rehashIndex)
	{
		rehashStep(d);
	}
	
	int index =(int)dictHashKey(d, key);
	int i = 0;
	for (i = 0; i < 2; i++)
	{	
		int j = index & d->ht[i].index;
		dictEntry *entry = d->ht[i].table[j];
		dictEntry *preventry = NULL;
		while (NULL != entry)
		{
			if (0 == dictCompareKey(d, entry->key, key))
			{
				if (NULL == preventry)
				{
					d->ht[i].table[j] = entry->next;
				}
				else
				{
					preventry->next = entry->next;
				}
				
				dictReleaseKey(d, entry);
				dictReleaseVal(d, entry);				
				reFree(entry);
				d->ht[i].used--;
				return OK;
			}
			
			preventry = entry;
			entry = entry->next;
		}
		if (-1 == d->rehashIndex)
		{
			break;
		}
	}
	return ERROR;
}

dictEntry *dictFind(dict *d, void *key)
{
	if (d == NULL || NULL == key || NULL == d->ht || 0 == d->ht[0].used)
	{
		return NULL;
	}
	if (-1 != d->rehashIndex)
	{
		rehashStep(d);
	}

	int index = dictHashKey(d, key);
	int i = 0;
	for (i = 0; i < 2; i++)
	{
		int j = index & d->ht[i].index;
		dictEntry *entry = d->ht[i].table[j];
		while (NULL != entry)
		{
			if (0 == dictCompareKey(d, entry->key, key))
			{
				return entry;
			}
			entry = entry->next;
		}
		if (-1 == d->rehashIndex)
		{
			break;
		}
	}
	return NULL;
}

dictEntry *dictRand(dict *d)
{
	if (NULL == d || NULL == d->ht || 0 == d->ht[0].used)
	{
		return NULL;
	}
	dictEntry *entry = NULL;
	int index = 0;
	if (-1 == d->rehashIndex)
	{
		do
		{
			index = rand() % d->ht[0].size;
			entry = d->ht[0].table[index];
		}while (NULL == entry);
	}
	else
	{
		do
		{
			index = rand() % (d->ht[0].size + d->ht[1].size);
			entry = (index >= d->ht[0].size) ? d->ht[1].table[index - d->ht[0].size] :  d->ht[0].table[index];
		}while(NULL == entry);
	}
	index = 0;
	dictEntry *tempEntry = entry;
	while (NULL != tempEntry)
	{
		index++;
		tempEntry = tempEntry->next;
	}
	index = rand() % index;
	while (index--)
	{
		entry = entry->next;
	}
	return entry;
}

static int dictClearHt(dict *d, dictht *ht)
{
	if (NULL == d || NULL == ht )
	{
		return ERROR;
	}
	if (NULL == ht->table)
	{
		return ERROR;
	}
	int i = 0;
	for (i = 0; i < ht->size && 0 < ht->used; i++)
	{
		dictEntry *entry = ht->table[i];
		dictEntry *next = NULL;
		while (NULL != entry)
		{
			next = entry->next;
			dictReleaseKey(d, entry);
			dictReleaseVal(d, entry);
			reFree(entry);
			ht->used--;
			entry = next;
		}
	}
	reFree(ht->table);
	dictResetHt(ht);
	return OK;
}

void dictClear(dict *d)
{
	if (NULL != d)
	{
		dictClearHt(d, &(d->ht[0]));
		dictClearHt(d, &(d->ht[1]));	
		d->iterator = 0;
		d->rehashIndex = -1;
	}
}

void dictRelease(dict *d)
{
	dictClear(d);
	reFree(d);
}

void *dictGetVal(dictEntry *entry)
{
	return (NULL != entry) ? (entry->val) : NULL;
}


int dictReplace(dict *d, void *key, void *val)
{
	if (NULL == d || NULL == key || NULL == val)
	{
		return ERROR;
	}
	if (OK == dictAdd(d, key, val))
	{
		return OK;
	}

	dictEntry *entry = dictFind(d, key);			
	if (NULL == entry)
	{
		return ERROR;
	}
	dictReleaseVal(d, entry);
	dictSetVal(d, entry, val);
	return OK;
}

void dictEnableResize(void)
{
	dict_resize = 1;
}
void dictDisableResize(void)
{
	dict_resize = 0;
}

dictIterator *dictGetIterator(dict *d)
{
	if (NULL == d)
	{
		return NULL;
	}
	dictIterator *it = reMalloc(sizeof(dictIterator));
	it->d = d;
	it->index = -1;
	it->table = 0;
	it->entry = NULL;
	it->nextEntry = NULL;
	return it;
}

void dictReleaseIterator(dictIterator *it)
{
	if (NULL != it && !(-1 == it->index && 0 == it->table))
	{
		it->d->iterator--;
	}
	reFree(it);	
}

dictEntry *dictNext(dictIterator *it)
{
	if (NULL == it)
	{
		return NULL;
	}
	
	while (1)
	{
		if (NULL == it->entry)
		{
			if (-1 == it->index && 0 == it->table)
			{
				it->d->iterator++;
			}

			it->index++;
			if (it->index >= it->d->ht[it->table].size)
			{
				if (-1 != it->d->rehashIndex && 0 == it->table)
				{
					it->index = 0;
					it->table++;
				}
				else
				{
					return NULL;
				}
			}
			it->entry = it->d->ht[it->table].table[it->index];
		}
		else
		{
			it->entry = it->nextEntry;
		}
		if (NULL != it->entry)
		{
			it->nextEntry = it->entry->next;
			return it->entry;
		}
	}
}


#ifdef _DEBUG_
void DisplayHt(dictht *ht, int flag)
{
	if (NULL == ht->table)
	{
		return;
	}
	
	int i = 0;
	for (i = 0; i < ht->size; i++)
	{
		dictEntry *pEntry = ht->table[i];
		int j = -1;
		while (NULL != pEntry)
		{
			j++;
			printf("table%d[%d][%d] = key = %s = val = %s\r\n", flag, i, j, (char *)(pEntry->key), (char *)(pEntry->val));
			pEntry = pEntry->next;	
		}
	}
}
void Display(dict *d)
{
	if (NULL == d)
	{
		return;
	}
	if (NULL != d->ht)
	{
		DisplayHt(d->ht, 0);

	}
	if (NULL != d->ht + 1)
	{
		DisplayHt(d->ht + 1, 1);
	}
}
void *CopyKey(void *privdata, void *key)
{	
	int len = strlen((char *)key);
	void *pBuf = reMalloc(len + 1);
	strcpy((char*)pBuf, (char*)key);
	return pBuf;
}
void *CopyVal(void *privdata, void *val)
{
	int len = strlen((char *)val);
	void *pBuf = reMalloc(len + 1);
	strcpy((char*)pBuf, (char*)val);
	return pBuf;;
}
void ReleaseKey(void *privdata,void *key)
{
	reFree(key);
}
void ReleaseVal(void *privdata,void *val)
{
	reFree(val);
}
int compareKey(void *privdata, void *key1, void *key2)
{
	int len1 = strlen((char *)key1);
	int len2 = strlen((char *)key2);
	int count = len1 > len2 ? len2 : len1;
	if (0 == (count = strncmp((char *)key1, (char *)key2, count)))
	{
		return len1 - len2;
	}
	return count;
}
int HashFunction(void *key)
{
	return dictGenHashFunction(key, strlen((char *)key));
}
char *atos(char **key, char **val, int data)
{
	*key = (char*)reMalloc(20);
	*val = (char*)reMalloc(20);
	int i = 0;
	do
	{
		int aux = data % 10;
		data = data/10;
		(*key)[i++] = '0' + aux;
	}while(0 != data);
	int j = strlen(*key) - 1;
	int k = 0;
	for(; k < j; k++, j--)
	{
		(*key)[k] ^= (*key)[j];
		(*key)[j] ^= (*key)[k];
		(*key)[k] ^= (*key)[j];		
	}
	strcpy(*val, *key);
}
int main(void)
{
	dictType type = 
	{
		//CopyKey,
		//CopyVal,
		NULL,
		NULL,
		ReleaseKey,
		ReleaseVal,
		compareKey,
		HashFunction
	};

	dict *d = dictCreate(&type, NULL);
	
	char *key, *val;
	int i = 0;
	for (; i < 100; i++)
	{
		atos(&key, &val, i);
		if (-1 == dictAdd(d, key, val))
		{
			return -1;			
		}
	}

	dictIterator *it = dictGetIterator(d);
	dictEntry *entry = NULL;
	while (NULL != (entry = dictNext(it)))
	{
		printf("%s\r\n", (char *)dictGetVal(entry));
	}
	dictReleaseIterator(it);
	printf("itersize = %d\r\n", d->iterator);
	
	printf("===========================================================================\r\n");
	
	if (d->ht[0].table != NULL)
	{
		printf("table[0].size = %d\r\n", d->ht[0].size);
		printf("table[0].used = %d\r\n", d->ht[0].used);
	}
	if (d->ht[1].table != NULL)
	{
		printf("table[1].size = %d\r\n", d->ht[1].size);
		printf("table[1].used = %d\r\n", d->ht[1].used);
	}
	dictRehashTime(d, 1);
	printf("-----------------------------------------------------------------------------\r\n");
	
	dictEnableResize();
	dictExpand(d, 1024);
	
	char *pBuf = (char *)reMalloc(40);
	strcpy(pBuf, "good morning");
	dictReplace(d, "2", pBuf);
	
	printf("rand val = %s\r\n", (char *)dictGetVal((dictEntry*)dictRand(d)));
	
	dictDelete(d, "1");
	printf("delete end = %s\r\n", (char *)dictGetVal((dictEntry*)dictFind(d, "1")));

	key = (char *)reMalloc(40);
	val = (char *)reMalloc(40);
	strcpy(key, "gjfhbkyfgkwefhgwkeyf");	
	strcpy(val, "171976286128365146351111");
	if (-1 == dictAdd(d, key, val))
	{
		return -1;			
	}
	
	printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\r\n");
	
	
	printf("rehashIndex = %d\r\n", d->rehashIndex);
	
	if (d->ht[0].table != NULL)
	{
		printf("table[0].size = %d\r\n", d->ht[0].size);
		printf("table[0].used = %d\r\n", d->ht[0].used);
	}
	if (d->ht[1].table != NULL)
	{
		printf("table[1].size = %d\r\n", d->ht[1].size);
		printf("table[1].used = %d\r\n", d->ht[1].used);
	}
	
	dictRelease(d);
	d = NULL;
	int sizeMemory = GetUsedMemory();
	printf("memory size = %d\r\n", sizeMemory);
	return 0;
}

#endif//_DEBUG_
