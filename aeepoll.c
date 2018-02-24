#include <sys/epoll.h>
#include "ae.h"
typedef struct API
{
	int epfd;
	struct epoll_event *events;
}API;

static int apiCreate(EventLoop *loop)
{
	API *api = (API *)reMalloc(sizeof(API));
	api->events = (struct epoll_event *)reMalloc(sizeof(struct epoll_event) * loop->size);
	api->epfd = epoll_create1(EPOLL_CLOEXEC);
	if (-1 == api->epfd)
	{
		reFree(api->events);
		reFree(api);
		loop->apiData = NULL;
		return -1;
	}
	loop->apiData = api;
	return 0;
}
static void apiRelease(EventLoop *loop)
{
	API *api = (API *)loop->apiData;
	close(api->epfd);
	reFree(api->events);
	reFree(api);
	loop->apiData = NULL;
}

static int apiAdd(EventLoop *loop, int fd, int mask)
{
	API *api = (API *)(loop->apiData);
	int op = (AE_NONE == loop->fileEvent[fd].mask) ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;
	
	struct epoll_event event;
	memset(&event, 0, sizeof(event));
	
	event.data.fd = fd;
	mask |= loop->fileEvent[fd].mask;
	if (mask & AE_READ)
	{
		event.events |= EPOLLIN;
	}
	if (mask & AE_WRITE)
	{
		event.events |= EPOLLOUT;
	}

	if (-1 == epoll_ctl(api->epfd, op, fd, &event))
	{
		return  -1;
	}
	return 0;
}

static int apiDelete(EventLoop *loop, int fd, int delmask)
{
	struct epoll_event event;
	memset(&event, 0, sizeof(event));
	event.data.fd = fd;

	int mask = (~delmask) & loop->fileEvent[fd].mask;
	if (mask & AE_READ)
	{
		event.events |= EPOLLIN;
	}
	if (mask & AE_WRITE)
	{
		event.events |= EPOLLOUT;
	}

	int op = (AE_NONE == mask) ? EPOLL_CTL_DEL : EPOLL_CTL_MOD;
	API *api = (API *)loop->apiData;
	if (-1 == epoll_ctl(api->epfd, op, fd, &event))
	{
		return -1;
	}
	return 0;
}

static int apiPoll(EventLoop *loop, struct timeval *timeout)
{
	API *api = (API *)loop->apiData;
	int time = timeout ? (timeout->tv_sec*1000 + timeout->tv_usec/1000) : -1;
	int count = epoll_wait(api->epfd, api->events, loop->size, time);
	if (0 < count)
	{
		int i = 0;
		for (i = 0; i < count; i++)
		{
			int mask = AE_NONE;
			if (api->events[i].events & EPOLLIN)
			{
				mask |= AE_READ;
			}
			if (api->events[i].events & EPOLLOUT)
			{
				mask |= AE_WRITE;
			}
			if (api->events[i].events & EPOLLERR)
			{
				mask |= AE_WRITE;
			}
			if (api->events[i].events & EPOLLHUP)
			{
				mask |= AE_WRITE;
			}
			loop->firedEvent[i].fd = api->events[i].data.fd;
			loop->firedEvent[i].mask = mask;
		}
	}
	return count;
}
