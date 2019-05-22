#include "ae.h"
typedef struct API
{
	fd_set readSet;
	fd_set writeSet;
	fd_set tempReadSet;
	fd_set tempWriteSet;
}API;

static int apiCreate(EventLoop *loop)
{
	API *api = (API *)reMalloc(sizeof(API));
	FD_ZERO(&api->readSet);
	FD_ZERO(&api->writeSet);	
	loop->apiData = api;
		
	return 0;
}
static void apiRelease(EventLoop *loop)
{
	reFree(loop->apiData);
	loop->apiData = NULL;
}

static int apiDelete(EventLoop *loop, int fd, int mask)
{
	API *api = loop->apiData;
	
	if (mask & AE_READ)
	{
		FD_CLR(fd, &api->readSet);
	}
	if (mask & AE_WRITE)
	{
		FD_CLR(fd, &api->writeSet);
	}
	return 0;
}

static int apiAdd(EventLoop *loop, int fd, int mask)
{
	API *api = loop->apiData;
	if (NULL == api)
	{
		return -1;
	}
	if (AE_NONE != mask & AE_READ)
	{
		FD_SET(fd, &api->readSet);
	}
	if (AE_NONE != mask & AE_WRITE)
	{
		FD_SET(fd, &api->writeSet);
	}
	return 0;
}

static int apiPoll(EventLoop *loop, struct timeval *timeout)
{
	API *api = loop->apiData;
	int max = loop->maxfd;
	if (NULL == api || 0 >= max)
	{
		return -1;
	}
	memcpy(&api->tempReadSet, &api->readSet, sizeof(fd_set));
	memcpy(&api->tempWriteSet, &api->writeSet, sizeof(fd_set));
	int rest = select(max + 1, &api->tempReadSet, &api->tempWriteSet, NULL, timeout);
	int count = 0;
	if (0 < rest)
	{	
		int i = 0;
		for (i = 0; i <= max; i++)
		{
			FileEvent *event = &loop->fileEvent[i];
			if (AE_NONE == event->mask)
			{
				continue;
			}
			do
			{
				if (AE_NONE != event->mask & AE_READ && FD_ISSET(i, &api->tempReadSet))
				{
					break;
				}
				if (AE_NONE != event->mask & AE_WRITE && FD_ISSET(i, &api->tempWriteSet))
				{
					break;
				}
			}while(0);
			loop->firedEvent[count].fd = i;
			loop->firedEvent[count].mask = event->mask;
			count++;
		}
		return count;
	}
	return -1;
}
