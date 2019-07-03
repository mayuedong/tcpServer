#include "eventLoopPool.h"
#include "eventLoop.h"
#include "threadEventLoop.h"
EventLoopPool::EventLoopPool(EventLoop *loop, int numThread)
: index_(0)
, numThread_(numThread)
, loop_(loop) {
}

void EventLoopPool::Start() {
  loop_->IsInLoop();
  for (int i = 0; i < numThread_; i++) {
    pool_.push_back(std::unique_ptr<ThreadEventLoop>(new ThreadEventLoop()));
    loops_.push_back(pool_[pool_.size()-1]->Start());
  }
}

EventLoop* EventLoopPool::GetLoop() {
  loop_->IsInLoop();
  if (index_ >= pool_.size()) {
    index_ = 0;
  }
  return loops_[index_++];
}
