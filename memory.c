#include "memory.h"
#include <pthread.h>

#define update_alloc_memory_size(size) \
	do \
	{ \
		int n = size; \
		if (size & (sizeof(long) - 1)) \
		n += sizeof(long) - (size & (sizeof(long) - 1)); \
		if (0 != memory_thread_safe) \
		{ \
			pthread_mutex_lock(&memory_mutex); \
			used_memory += n; \
			pthread_mutex_unlock(&memory_mutex); \
		} \
		else \
		used_memory += n; \
	}while(0);

#define update_free_memory_size(size) \
	do \
	{ \
		int n = size; \
		if (size & (sizeof(long) - 1)) \
		n += sizeof(long) - (size & (sizeof(long) - 1)); \
		if (0 != memory_thread_safe) \
		{ \
			pthread_mutex_lock(&memory_mutex); \
			used_memory -= n; \
			pthread_mutex_unlock(&memory_mutex); \
		} \
		else \
		used_memory -= n; \
	}while(0);

#define PREFIX_SIZE sizeof(int)

static int used_memory = 0;
static int memory_thread_safe = 0;
pthread_mutex_t memory_mutex = PTHREAD_MUTEX_INITIALIZER;

static void ProcessError(int size)
{
	fprintf(stderr, "allocate buffer error and buffer size = %d\n", size);
	fflush(stderr);
	abort();
}

void *reMalloc(int size)
{
	void *ptr = malloc(size + PREFIX_SIZE);
	if (NULL == ptr)
	{
		ProcessError(size);
	}
	memset(ptr, 0, size + PREFIX_SIZE);
	*((int*)ptr) = size;
	update_alloc_memory_size(size + PREFIX_SIZE);
	return (char*)ptr + PREFIX_SIZE;
}

void *reCalloc(int size)
{
	void *ptr = calloc(1, size+PREFIX_SIZE);
	if (NULL == ptr)
	{
		ProcessError(size);
	}
	*((int*)ptr) = size;
	update_alloc_memory_size(size + PREFIX_SIZE);
	return (char*)ptr + PREFIX_SIZE;
}

void *reFree(const void *ptr)
{
	if (NULL != ptr)
	{
		char *tempPtr = (char*)ptr - PREFIX_SIZE;
		int size = *((int*)tempPtr);
		free(tempPtr);
		update_free_memory_size(size + PREFIX_SIZE);
	}
}
void *reRealloc(void *ptr, int size)
{   
	if (NULL == ptr)
	{
		return reMalloc(size);
	}
	void *oldPtr = (char*)ptr - PREFIX_SIZE;
	int oldSize = *((int*)oldPtr);
	void *newPtr = realloc(oldPtr, size + PREFIX_SIZE);
	if (NULL == newPtr)
	{
		ProcessError(size);
	}
	*((int*)newPtr) = size;
	update_free_memory_size(oldSize + PREFIX_SIZE);
	update_alloc_memory_size(size + PREFIX_SIZE);
	return (char*)newPtr + PREFIX_SIZE;
}



int GetBufSize(const void *ptr)
{
	if (NULL == ptr)
	{
		return -1;
	}
	char *tempPtr = (char*)ptr - PREFIX_SIZE;
	int size = *((int*)tempPtr);
	return size;
}

int GetUsedMemory()
{
	int size = 0;
	if (0 != memory_thread_safe)
	{
		pthread_mutex_lock(&memory_mutex);
		size = used_memory;
		pthread_mutex_unlock(&memory_mutex);
	}
	else
	{
		size = used_memory;
	}
	return size;
}
