#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#define _DEBUG_


#define ERROR   -1
#define OK      0

#define	LOG_LEN			512
#define	LOG_DEBUG		0
#define	LOG_INFO		1
#define	LOG_NOTICE		2
#define	LOG_WARNING		3


void Log(int level, char *fmt, ...);
long long timeUs();
long long timeMs();

#endif//_COMMON_H_
