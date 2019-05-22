#include "memory.h"
#include "sds.h"

int sdsLength(sds s)
{
	if (NULL == s)
	{
		return 0;
	}
	return ((sdsHead*)(s - sizeof(sdsHead)))->len;
}

int sdsFreedom(sds s)
{
	if (NULL == s)
	{
		return 0;
	}
	return ((sdsHead*)(s - sizeof(sdsHead)))->freed;
}

int sdsSize(sds s)
{
	if (NULL == s)
	{
		return 0;
	}
	sdsHead *pHead = (void*)(s - sizeof(sdsHead));
	return pHead->freed + pHead->len + 1 + sizeof(sdsHead);
}

sds sdsNewLen(const void *pBuf, int len)
{
	if (NULL == pBuf || 0 > len)
	{
		return NULL;
	}
	sdsHead *pHead = reMalloc(sizeof(sdsHead) + len + 1);
	memcpy(pHead->buf, pBuf, len);
	pHead->buf[len] = '\0';
	pHead->freed = 0;
	pHead->len = len;
	return pHead->buf;
}

sds sdsNewEmpty()
{
	return sdsNewLen("",0);
}

sds sdsNew(const char *pBuf)
{
	if (NULL == pBuf)
	{
		return NULL;
	}
	return sdsNewLen(pBuf, strlen(pBuf));
}

sds sdsCopy(const sds s)
{
	if (NULL == s)
	{
		return NULL;
	}
	return sdsNewLen(s, sdsLength(s));
}

void sdsClear(sds s)
{
	if (NULL == s)
	{
		return;
	}
	sdsHead *pHead = (void*)(s - sizeof(sdsHead));
	pHead->buf[0] = '\0';
	pHead->freed += pHead->len;
	pHead->len = 0;
}

void sdsRelease(sds s)
{
	if (NULL == s)
	{
		return;
	}
	sdsHead *pHead = (void*)(s - sizeof(sdsHead));
	reFree(pHead);
}

void sdsUpdateLen(sds s)
{
	if (NULL == s)
	{
		return;
	}
	sdsHead *pHead = (void*)(s - sizeof(sdsHead));
	int len = strlen(s);
	pHead->freed += pHead->len - len;
	pHead->len = len;
}

sds sdsAddRoom(sds s, int addLen)
{
	if (NULL == s)
	{
		return NULL;
	}
	sdsHead *pHead = (void*)(s - sizeof(sdsHead));
	if (pHead->freed >= addLen)
	{
		return s;			
	}
	int len = pHead->len;
	int newLen = len + addLen;
	newLen = newLen < _SDS_MAX_SIZE_ ? newLen * 2 : newLen + _SDS_MAX_SIZE_;
	
	sdsHead *newPHead = reRealloc(pHead, sizeof(sdsHead) + newLen + 1);
	newPHead->freed = newLen - len;
	return newPHead->buf;
}

sds sdsRemoveRoom(sds s)
{
	if (NULL == s)
	{
		return NULL;
	}
	sdsHead *pHead = (void*)(s - sizeof(sdsHead));
	sdsHead *newPHead = reRealloc(pHead, sizeof(sdsHead) + pHead->len + 1);
	newPHead->freed = 0;
	return newPHead->buf;;
}

sds sdsGrowZero(sds s, int len)
{
	if (NULL == s || 0 >= len)
	{
		return NULL;
	}
	sdsHead *pHead = (void*)(s - sizeof(sdsHead));
	int oldLen = pHead->len;
	sds news = sdsAddRoom(s, len - oldLen);

	pHead = (void*)(news - sizeof(sdsHead));
	memset(news+ oldLen, 0, len - oldLen + 1);
	int auxLen = pHead->freed + pHead->len;
	pHead->len = len;
	pHead->freed = auxLen - len;
	return pHead->buf;
}

sds CatSdsLen(sds to, const void* from, int len)
{
	if (NULL == to || NULL == from || 0 >= len)
	{
		return NULL;
	}
	sds newto = sdsAddRoom(to, len);

	sdsHead *pHead = (void*)(newto - sizeof(sdsHead));
	int newtoLen = pHead->len;
	memcpy(newto + newtoLen, from, len);
	newto[newtoLen + len] = '\0';
	pHead->len = newtoLen + len;
	pHead->freed -= len;
	return newto;
}

sds sdsCatBuf(sds to, const char *from) 
{
	if (NULL != to && NULL != from)
	{
		return CatSdsLen(to, from, strlen(from));
	}
	return NULL;
}

sds sdsCatSds(sds to, const sds from) 
{
	if (NULL != to && NULL != from)
	{
		return CatSdsLen(to, from, sdsLength(from));
	}
	return NULL;
}

sds sdsCopyBufLen(sds to, const void *from, int len)
{
	if (NULL == to || NULL == from || 0 >= len)
	{
		return NULL;
	}
	sdsHead *pHead = (void*)(to - sizeof(sdsHead));
	int allLen = pHead->len + pHead->freed;
	if (allLen < len)
	{
		to = sdsAddRoom(to, len - pHead->len);
		pHead = (void*)(to - sizeof(sdsHead));
		allLen = pHead->len + pHead->freed;
	}
	memcpy(to, from, len);
	to[len] = '\0';
	pHead->len = len;
	pHead->freed = allLen - len;
	return to;
}

sds sdsCopyBuf(sds to, const char *from)
{
	if (NULL != to && NULL != from)
	{
		return sdsCopyBufLen(to, from, strlen(from));
	}
}

sds sdsCopySds(sds to, sds from)
{
	if (NULL != to && NULL != from)
	{
		return sdsCopyBufLen(to, from, sdsLength(from));
	}
}


void sdsLow(sds str)
{
	if (NULL != str)
	{
		int  len= sdsLength(str);
		int i = 0;
		for (i = 0; i < len; i++)
		{
			str[i] = tolower(str[i]);
		}
	}
}
void sdsUp(sds str)
{
	if (NULL != str)
	{
		int len = sdsLength(str);
		int i = 0;
		for (i = 0; i < len; i++)
		{
			str[i] = toupper(str[i]);
		}
	}
}

int sdsCompare(const sds s1, const sds s2)
{
	if (NULL == s1 && NULL != s2)
	{
		return -1;
	}
	if (NULL != s1 && NULL == s2)
	{
		return 1;
	}
	if (NULL == s1 && NULL == s2)
	{
		return 0;
	}

	int len1 = sdsLength(s1);
	int len2 = sdsLength(s2);
	int minLen = len1 > len2 ? len2 : len1;
	int ret = strncmp(s1, s2, minLen);
	if (0 == ret)
	{
		return len1 - len2;
	}
	return ret;
}

sds sdsAtos(long long data)
{
	char buf[32];
	char *p = buf + 31;
	long long temp = data < 0 ? -data : data;
	do
	{
		*p-- = '0' + (temp % 10);
		temp /= 10;
	}while (0 < temp);
	
	if (0 > data)
	{
		*p-- = '-';
	}
	p++;
	return  sdsNewLen(p, 32 - (p - buf));
}

sds sdsTrim(sds str, const char *buf)
{
	if (NULL == str || NULL == buf)
	{
		return NULL;
	}
	sdsHead *pHead = (void *)(str - sizeof(sdsHead));

	char *start = pHead->buf;
	char *end = pHead->buf + pHead->len - 1;
	while (start < end && NULL != strchr(buf, *start))
	{
		start++;
	}
	while (end > pHead->buf && NULL != strchr(buf, *end))
	{
		end--;
	}
	int len = end < start ? 0 : end - start + 1;
	memmove(pHead->buf, start, len);
	pHead->buf[len] = '\0';
	pHead->freed = pHead->freed + pHead->len - len;
	pHead->len = len;
	return pHead->buf;
}

sds sdsMove(sds str, int start, int end)
{
	if (NULL == str)
	{
		return NULL;
	}
	
	sdsHead *pHead = (void *)(str - sizeof(sdsHead));
	int len = pHead->len;
	if (0 >= len)
	{
		return str;
	}
	
	if (0 > start)
	{
		start += len;
		if (0 > start)
		{
			start = 0;
		}
	}
	if (0 > end)
	{
		end += len;
		if (0 > end)
		{
			end = 0;
		}
	}
	
	int newLen = start > end ? 0 : end - start + 1;
	if (0 != newLen)
	{
		if (start >= len)
		{
			newLen = 0;
		}
		else if (end >= len)
		{
			end = len - 1;
			newLen = start > end ? 0 : end - start + 1;
		}
	}

	if (0 != newLen && 0 != start)
	{
		memmove(pHead->buf, pHead->buf + start, newLen);
	}
	pHead->buf[newLen] = '\0';
	pHead->freed = pHead->freed + pHead->len - newLen;
	pHead->len = newLen;
	return str;
}

void sdsReleaseSplit(sds *pArr, int count)
{
	if (NULL == pArr)
	{
		return;
	}
	int i = 0;
	for (i = 0; i < count; ++i)
	{
		sdsRelease(pArr[i]);
	}
	reFree(pArr);
}

sds *sdsSplit(const char *str1, int len1, const char *str2, int len2, int *pCount)
{
	if (NULL == str1 || NULL == str2 || NULL == pCount ||0 >= len1 || 0 >= len2)
	{
		return NULL;
	}

	int size = 5, start = 0, i = 0, num = 0;
	sds *pArr = reMalloc(sizeof(sds) * size);

	for (i = 0; i <= len1 - len2; i++)
	{
		if (size < num + 2)
		{
			size *= 2;
			sds *pTemp = reRealloc(pArr, size * sizeof(sds));
			pArr = pTemp;
		}

		if (0 == memcmp(str1 + i, str2, len2))
		{
			if (i != start)
			{
				pArr[num] = sdsNewLen(str1 + start, i - start );
				num++;
			}
			start = i + len2;
			i = start - 1;
		}
	}

	if (len1 != start)
	{
		pArr[num] = sdsNewLen(str1 + start, len1 - start);
		num++;
	}

	*pCount = num;
	return pArr;
}

void sdsIncressLen(sds str, int len)
{
	if (NULL != str)
	{
		sdsHead *pHead = (void *)(str - sizeof(sdsHead));
		if (len <= pHead->freed)
		{
			pHead->freed -= len;
			pHead->len += len;
			str[pHead->len] = '\0';
		}
	}
}

#ifdef _DEBUG_

static void Display(sds s)
{
	if (NULL == s)
	{
		return;
	}
	sdsHead *pHead = (sdsHead *)(s - sizeof(sdsHead));
	printf("size = %d\r\n", pHead->len);
	printf("free = %d\r\n", pHead->freed);
	printf("buf = %s\r\n", pHead->buf);
}

int main(void)
{
	char *pBuf = reMalloc(100);
	strcpy(pBuf, "Test sdsNew");
	sds from = sdsNew(pBuf);
	if (NULL != from)
	{
		Display(from);//11,0,Test sdsNew	
	}
	
	sdsLow(from);//11, 0,test sdsNew
	Display(from);
	
	sds to = sdsNewEmpty();
	if (NULL != to)
	{
		Display(to);//0,0,NULL
		int size = sdsGetSize(to);
		printf("size of to = %d\r\n", size);//9 = 4 + 4 + 1
	}
	
	if (NULL != (to = sdsGrowZero(to, 50)))
	{
		int size = sdsGetSize(to);
		printf("size of to = %d\r\n", size);//109 = 9 + 100;
		Display(to);//50,50,NULL
	}
	
	sdsUpdateLen(to);
	Display(to);//0, 100, NULL

	if (NULL !=(to = sdsCopyBuf(to, pBuf)))
	{
		Display(to);//11, 89, Test sdsNew
	}
	
	if (NULL !=(to = sdsCopySds(to, from)))
	{
		Display(to);//11, 89, test sdsNew
	}
	
	sds temp = sdsCopy(from);
	if (NULL != temp)
	{
		sdsUp(temp);
		Display(temp);//11, 0, TEST sdsNew
		sdsClear(temp);
		Display(temp);//0, 11, NULL
		sdsRelease(temp);
		temp = NULL;
	}
	
	if (NULL != (to = sdsRemoveRoom(to)))
	{
		Display(to);//11, 0, test sdsNew
	}

	memset(pBuf, 0, strlen(pBuf));
	strcpy(pBuf, " Successful!");
	if (NULL != (to = sdsCatBuf(to, pBuf)))
	{
		Display(to);//23, 23, test sdsNew Successful!
	}
	
	if (NULL != (to = sdsCatSds(to, from)))
	{
		Display(to);//34, 12, test sdsNew Successful!test sdsNew
		sdsRelease(from);
		from = NULL;
	}
	
	long long data = -12345678;
	sds SdsNum = sdsAtos(data);
	Display(SdsNum);
	sdsRelease(SdsNum);
	SdsNum = NULL;
	
	data = 12345678910LL;
	SdsNum = sdsAtos(data);
	Display(SdsNum);
	sdsRelease(SdsNum);
	SdsNum = NULL;
	
	if (NULL != (to = sdsMove(to, 0, -8)))
	{
		Display(to);//test sdsNew Successful!test 
	}
	if (NULL != (to = sdsTrim(to, "tst")))
	{
		Display(to);//est sdsNew Successful!te
	}
	if (NULL != (to = sdsTrim(to, "n test")))
	{
		Display(to);//wsds Successful!
	}
	
	memset(pBuf, 0, strlen(pBuf));
	strcpy(pBuf, "  Example:    Last week I want to a sealt. I have a very good sealt but   Idon`tNot");
	if (NULL != (to = sdsCopyBuf(to, pBuf)))
	{
		Display(to);
	}
	
	int count = -1;
	sds *pSds = sdsSplit(to,strlen(pBuf), " ", 1, &count);
	if (NULL != pSds)
	{
		int i = 0;
		while (i < count)
		{
			printf("\r\n");
			Display(pSds[i]);
			i++;
		}
		sdsReleaseSplit(pSds,&count);
		printf("count = %d\r\n", count);
		sdsRelease(to);
		to = NULL;
	}
	reFree(pBuf);
	pBuf = NULL;
	int sizeMemory = GetUsedMemory();
	printf("memory size = %d\r\n", sizeMemory);
	
	return 0;
}

#endif//_DEBUG_
