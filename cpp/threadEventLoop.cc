#include "threadEventLoop.h"
#include "eventLoop.h"
#include <functional>
ThreadEventLoop::ThreadEventLoop()
: loop_(NULL)
, thread_(std::bind(&ThreadEventLoop::run, this))
, mutex_()
, cond_(mutex_){
}

EventLoop* ThreadEventLoop::Start() {
  thread_.Start();
  MutexLock lock(mutex_);
  while (NULL == loop_) {
    cond_.Wait();
  }
  return loop_;
}

void ThreadEventLoop::run() {
  EventLoop loop;
  {
    MutexLock lock(mutex_);
    loop_ = &loop;
    cond_.Notify();
  }
  loop.Loop();
}
