#ifndef _THREAD_EVENT_LOOP_H_
#define _THREAD_EVENT_LOOP_H_

#include "thread.h"
#include "mutex.h"
#include "cond.h"
class EventLoop;
class ThreadEventLoop {
public:
  ThreadEventLoop();
  EventLoop *Start();
private:
  void run();
  EventLoop *loop_; 
  Thread thread_;
  Mutex mutex_;
  Cond cond_;
};
#endif//_THREAD_EVENT_LOOP_H_
