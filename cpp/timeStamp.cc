#include "timeStamp.h"

#include <sys/time.h>
#include <stdio.h>
#include <inttypes.h>

std::string TimeStamp::String() const{
  char buf[32] = {0};
  int64_t seconds = us_ / 1e6;
  int64_t microseconds = us_ % static_cast<int64_t>(1e6);
  snprintf(buf, sizeof(buf)-1, "%" PRId64 ".%06" PRId64 "", seconds, microseconds);
  return buf;
}

std::string TimeStamp::Format() const{
  char buf[64] = {0};
  time_t seconds = static_cast<time_t>(us_ / 1e6);
  struct tm tm_time;
  gmtime_r(&seconds, &tm_time);
  int microseconds = static_cast<int>(us_ % static_cast<int64_t>(1e6));
  snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%06d",
           tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
           tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
           microseconds);
  return buf;
}

TimeStamp TimeStamp::Now() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  int64_t seconds = tv.tv_sec;
  return TimeStamp(seconds * 1e6 + tv.tv_usec);
}
