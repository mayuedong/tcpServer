#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "common.h"

void *reMalloc(int size);
void *reCalloc(int size);
void *reRealloc(void *ptr, int size);
void *reFree(const void *ptr);
int GetUsedMemory();
int GetBufSize(const void *ptr);

#endif//_MEMORY_H_
