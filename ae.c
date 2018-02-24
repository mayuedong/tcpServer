#include "memory.h"
#include "ae.h"
#include "aeepoll.c"
#include <errno.h>

EventLoop *aeCreate(int size)
{
	if (0 >= size)
	{
		return NULL;
	}

	EventLoop *loop = (EventLoop *)reMalloc(sizeof(EventLoop));
	FileEvent *fileEvent = (FileEvent *)reMalloc(sizeof(FileEvent) * size);
	FiredEvent *fireEvent = (FiredEvent *)reMalloc(sizeof(TimeEvent) * size);
	
	if (NULL == loop || NULL == fileEvent || NULL == fireEvent)
	{
		goto err;
	}
	
	loop->fileEvent = fileEvent;
	loop->firedEvent = fireEvent;
	loop->size = size;
	loop->maxfd = -1;
	loop->stop = 0;
	loop->timeEventHead = NULL;
	loop->lastTime = time(NULL);
	loop->nextTimeEventId = 0;
	loop->beforeSleep = NULL;
	
	if (-1 == apiCreate(loop))
	{
		goto err;
	}
	return loop;
	
err:	
	reFree(fireEvent);
	reFree(fileEvent);
	reFree(loop);
	return NULL;
}

void aeRelease(EventLoop *loop)
{
	if (NULL != loop)
	{
		apiRelease(loop->apiData);
		reFree(loop->firedEvent);
		reFree(loop->fileEvent);
		reFree(loop);
	}
}

void aeStop(EventLoop *loop)
{
	if (NULL != loop)
	{
		loop->stop = 1;
	}
}

int aeCreateFileEvent(EventLoop *loop, int fd, int mask, void *proc, void *clientData)
{
	if (NULL == loop || 0 > fd || AE_NONE == mask)
	{
		return ERROR;
	}
	if (loop->size <= fd)
	{
		errno = ERANGE;
		return ERROR;
	}
	
	if (-1 == apiAdd(loop, fd, mask))
	{
		return ERROR;
	}
	
	FileEvent *event = loop->fileEvent + fd;
	if (mask & AE_READ)
	{
		event->rProc = proc;
	}
	if (mask & AE_WRITE)
	{
		event->wProc = proc;
	}
	
	event->mask |= mask;
	event->clientData = clientData;
	
	if (fd > loop->maxfd)
	{
		loop->maxfd = fd;
	}
	return OK;
}

int aeDeleteFileEvent(EventLoop *loop, int fd, int mask)
{
	if (NULL == loop ||0 > fd  || loop->size <= fd ||AE_NONE == mask)
	{
		return ERROR;
	}
	
	FileEvent *event = loop->fileEvent + fd;
	apiDelete(loop, fd, mask);
	event->mask &= ~mask;
	if (loop->maxfd == fd && AE_NONE == event->mask)
	{
		int i = loop->maxfd - 1;
		while (0 <= i)
		{
			if (AE_NONE != loop->fileEvent[i].mask)
			{
				break;
			}
			i--;
		}
		loop->maxfd = i;
	}
	return OK;
}

int aeGetFileEvent(EventLoop *loop, int fd)
{
	if (NULL == loop || 0 > fd || loop->maxfd <= fd)
	{
		return -1;
	}
	FileEvent *fileEvent = &loop->fileEvent[fd];
	return fileEvent->mask;
}
static void GetNowTime(long long *sec, long long *ms)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	*sec = tv.tv_sec;
	*ms = tv.tv_usec / 1000;
}
static void AddTimeToNow(long long fromMs, long long *toSec, long long *toMs)
{
	long long sec = 0;
	long long ms = 0;
	GetNowTime(&sec, &ms);
	sec += fromMs / 1000;
	ms += fromMs % 1000;
	if (1000 <= ms)
	{
		ms -= 1000;
		sec++;
	}
	*toSec = sec;
	*toMs = ms;
}

int aeCreateTimeEvent(EventLoop *loop, TimeEventProc *timeProc, FinalizerEventProc *finalProc, void *clientData, long long ms)
{
	if (NULL == loop || NULL == timeProc || NULL == finalProc || NULL == clientData)
	{
		return ERROR;
	}

	TimeEvent *timeEvent = (TimeEvent *)reMalloc(sizeof(TimeEvent));
	if (NULL == timeEvent)
	{
		return ERROR;
	}
		
	timeEvent->clientData = clientData;
	timeEvent->finalProc = finalProc;
	timeEvent->timeProc = timeProc;
	timeEvent->id = loop->nextTimeEventId++;
	AddTimeToNow(ms, &timeEvent->sec, &timeEvent->ms);
	
	timeEvent->next = loop->timeEventHead;
	loop->timeEventHead = timeEvent;
	return timeEvent->id;
}

int aeDeleteTimeEvent(EventLoop *loop, int id)
{
	if (NULL == loop || 0 > id || loop->nextTimeEventId <= id)
	{
		return ERROR;
	}
	
	TimeEvent *timeEvent = loop->timeEventHead;
	TimeEvent *auxPrev = NULL;
	while (NULL != timeEvent)
	{
		if (id == timeEvent->id)
		{
			if (NULL == auxPrev)
			{
				loop->timeEventHead = timeEvent->next;
			}
			else
			{
				auxPrev->next = timeEvent->next;
			}
			if (NULL != timeEvent->finalProc)
			{
				timeEvent->finalProc(loop, timeEvent->clientData);
			}
			reFree(timeEvent);
			return OK;
		}
		auxPrev = timeEvent;
		timeEvent = timeEvent->next;
	}
	return ERROR;
}

static TimeEvent *SearchNearTimeEvent(EventLoop *loop)
{
	TimeEvent *timeEvent = loop->timeEventHead;
	TimeEvent *auxEvent = NULL;
	while (NULL != timeEvent)
	{
		if (NULL == auxEvent || auxEvent->sec > timeEvent->sec || auxEvent->sec == timeEvent->sec && auxEvent->ms > timeEvent->ms)
		{
			auxEvent = timeEvent;
		}
		timeEvent = timeEvent->next;
	}
	return auxEvent;
}

static int ProcessTimeEvent(EventLoop *loop)
{
	time_t now = time(NULL);
	if (now < loop->lastTime)
	{
		TimeEvent *event = loop->timeEventHead;
		while (NULL != event)
		{
			event->sec = 0;
			event->ms = 0;
			event = event->next;
		}
	}
	
	loop->lastTime = now;
	TimeEvent *event = loop->timeEventHead;
	int count = 0;
	while (NULL != event)
	{
		if (event->id >= loop->nextTimeEventId)
		{
			event = event->next;
			continue;
		}
		
		long long sec = 0, ms = 0;
		GetNowTime(&sec, &ms);
		if (sec > event->sec || sec == event->sec && ms >= event->ms)
		{
			if (NULL == event->timeProc)
			{
				return ERROR;
			}
			
			int rest = event->timeProc(loop, event->clientData, event->id);
			count++;
			if (AE_NOMORE == rest)
			{
				aeDeleteTimeEvent(loop, event->id);
			}
			else
			{
				AddTimeToNow(rest, &event->sec, &event->ms);
			}
			event = loop->timeEventHead;
		}
		else
		{
			event = event->next;
		}
	}
	return count;
}

int aeProcessEvent(EventLoop *loop, int type)
{
	if (NULL ==loop || !(AE_FILE_EVENT & type) && !(AE_TIME_EVENT & type))
	{
		return ERROR;
	}
	int processed = 0;
	if (-1 < loop->maxfd || (AE_TIME_EVENT & type) && !(AE_DONT_WAIT & type))
	{	
		TimeEvent *event = NULL;
		struct timeval timeout;
		struct timeval *pTimeout = &timeout;
		if ((AE_TIME_EVENT & type) && !(AE_DONT_WAIT & type))
		{
			event = SearchNearTimeEvent(loop);
		}
		if (NULL != event)
		{
			long long nowSec = 0;
			long long nowMs = 0;
			GetNowTime(&nowSec, &nowMs);
			timeout.tv_sec = event->sec - nowSec;
			long long auxMs = event->ms - nowMs;
			if (0 > auxMs)
			{
				--timeout.tv_sec;
				auxMs += 1000;
			}
			timeout.tv_usec = auxMs * 1000;
			if (0 > timeout.tv_sec)
			{
				timeout.tv_sec = 0;
			}
			if (0 > timeout.tv_usec)
			{
				timeout.tv_usec = 0;
			}
		}
		else
		{
			if (AE_DONT_WAIT & type)
			{
				timeout.tv_sec = 0;
				timeout.tv_usec = 0;
			}
			else
			{
				pTimeout = NULL;
			}
		}
		
		processed = apiPoll(loop, pTimeout);	
		int i = 0;
		for (i = 0; i < processed; i++)
		{
			int fd = loop->firedEvent[i].fd;
			int mask = loop->firedEvent[i].mask;
			FileEvent *fileEvent = &loop->fileEvent[fd];
			int flag = 0;
			if (fileEvent->mask & mask & AE_READ)
			{
				fileEvent->rProc(loop, fileEvent->clientData, fd, fileEvent->mask);
				flag = 1;
			}
			if (fileEvent->mask & mask & AE_WRITE)
			{
				if (0 == flag || fileEvent->rProc != fileEvent->wProc)
				{
					fileEvent->wProc(loop, fileEvent->clientData, fd, fileEvent->mask);
				}
			}
		}
	}
	if (AE_TIME_EVENT & type)
	{
		processed += ProcessTimeEvent(loop);
	}
	return processed;
}

int Wait(int mask, int fd, long ms)
{

}

void aeMain(EventLoop *loop)
{
	loop->stop = 0;
	while (1 != loop->stop)
	{
		if (NULL != loop->beforeSleep)
		{
			loop->beforeSleep(loop);
		}
		aeProcessEvent(loop, AE_ALL_EVENT);
	}
}

void aeSetBeforeSleepProc(EventLoop *loop, BeforSleepProc *beforeSleep)
{
	if (NULL != loop)
	{
		loop->beforeSleep = beforeSleep;
	}
}
