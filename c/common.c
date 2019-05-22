#include "common.h"
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>

void Log(int level, char *fmt, ...)
{
	FILE *fp = fopen("log", "a");
	if (NULL == fp)
		return;
	
	char msg[LOG_LEN];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(msg, sizeof(msg), fmt, ap);
	va_end(ap);

	char buf[64];
	char *c = ".-*#";
	time_t now = time(NULL);
	strftime(buf, sizeof(buf), "%Y-%m-%d	%H:%M:%S", localtime(&now));
	fprintf(fp, "[%d]	%s	%c	%s\r\n", (int)getpid(), buf, c[level], msg);
	fflush(fp);
	fclose(fp);
}

long long timeUs()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	long long now = tv.tv_sec * 1000 * 1000 + tv.tv_usec;
	return now;
}

long long timeMs()
{
	return timeUs() / 1000;
}

#ifdef _DEBUG_
int main(void)
{
	Log(0, "test log = %s", "myd is a good boy!");
	Log(1, "test log = %d", 9527);
	Log(2, "test log = %f", 3.1415926);	
	Log(3, "test log = %c", 'q');	
	Log(0, "ms = %ld", timeUs());
	Log(1, "us = %ld", timeUs());	

	return 0;
}
#endif
