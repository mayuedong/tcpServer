#ifndef _AE_H_
#define _AE_H_

#include "common.h"

#define	AE_NONE	0
#define	AE_READ	1
#define	AE_WRITE			2

#define	AE_FILE_EVENT	1
#define	AE_TIME_EVENT	2
#define	AE_ALL_EVENT	3
#define	AE_DONT_WAIT	4

#define	AE_NOMORE	-1

struct EventLoop;
typedef void FileEventProc(struct EventLoop *loop, void *clientData, int fd, int mask);
typedef int TimeEventProc(struct EventLoop *loop, void *clientData, int id);
typedef void FinalizerEventProc(struct EventLoop *loop, void *clientData);
typedef void BeforSleepProc(struct EventLoop *loop);

typedef struct FileEvent
{
	int mask;
	FileEventProc *rProc;
	FileEventProc *wProc;
	void *clientData;
}FileEvent;

typedef struct FiredEvent
{
	int fd;
	int mask;
	
}FiredEvent;

typedef struct TimeEvent
{
	int id;
	long long sec;
	long long ms;
	TimeEventProc *timeProc;
	FinalizerEventProc *finalProc;
	void *clientData;
	struct TimeEvent *next;
}TimeEvent;

typedef struct EventLoop
{
	int maxfd;
	int size;
	FileEvent *fileEvent;
	FiredEvent *firedEvent;
	void *apiData;
	TimeEvent *timeEventHead;
	int nextTimeEventId;
	time_t lastTime;
	int stop;
	BeforSleepProc *beforeSleep;
} EventLoop;


EventLoop *aeCreate(int size);
void aeRelease(EventLoop *loop);
void aeStop(EventLoop *loop);
int aeCreateFileEvent(EventLoop *loop, int fd, int mask, void *proc, void *clientData);
int aeDeleteFileEvent(EventLoop *loop, int fd, int mask);
int aeGetFileEvent(EventLoop *loop, int fd);
int aeCreateTimeEvent(EventLoop *loop, TimeEventProc *timeProc, FinalizerEventProc *finalProc, void *clientData, long long ms);
int aeDeleteTimeEvent(EventLoop *loop, int id);
int aeProcessEvent(EventLoop *loop, int type);
void aeMain(EventLoop *loop);
void aeSetBeforeSleepProc(EventLoop *loop, BeforSleepProc *beforeSleep);


#endif//_AE_H_

