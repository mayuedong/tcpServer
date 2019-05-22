
#ifndef _SDS_H_
#define _SDS_H_

#include "common.h"

#define _SDS_MAX_SIZE_ (1024 * 1024)

typedef char* sds;

typedef struct sdsHead
{
	int len;
	int freed;
	char buf[];
}sdsHead;

int sdsSize(sds s);
int sdsLength(sds s);
int sdsFreedom(sds s);

sds sdsNewEmpty();
sds sdsNew(const char *pBuf);
sds sdsNewLen(const void *pBuf, int len);

void sdsClear(sds s);
void sdsRelease(sds s);
void sdsResetLen(sds s);

sds sdsAddRoom(sds s, int addLen);
sds sdsRemoveRoom(sds s);
sds sdsGrowZero(sds s, int len);
void sdsUpdateLen(sds s);
void sdsIncressLen(sds s, int len);

sds CatSdsLen(sds to, const void* from, int len);
sds sdsCatBuf(sds to, const char *from) ;
sds sdsCatSds(sds to, const sds from);

sds sdsCopy(const sds s);
sds sdsCopyBuf(sds to, const char *from);
sds sdsCopySds(sds to, sds from);

int sdsCompare(const sds s1, const sds s2);

void sdsLow(sds str);
void sdsUp(sds str);
sds sdsAtos(long long data);

sds sdsTrim(sds str, const char *buf);
sds sdsMove(sds str, int start, int end);
sds *sdsSplit(const char *str1, int len1, const char *str2, int len2, int *pCount);
void sdsReleaseSplit(sds *pArr, int count);

void sdsIncressLen(sds str, int len);

#endif //_SDS_H_

