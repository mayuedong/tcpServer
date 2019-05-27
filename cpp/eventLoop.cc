#include "eventLoop.h"
#include "channel.h"
#include "poller.h"
#include <sys/eventfd.h>
#include <unistd.h>

EventLoop::EventLoop()
: loopping_(false)
, handing_(false)
, wake_(eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC))
, tid_(GetTid())
, poller_(new Poller()) 
, wakeChannel_(new Channel(this, wake_)) {
  wakeChannel_->SetRead(std::bind(&EventLoop::handleRead, this));
  wakeChannel_->AddReadEvent();
}

bool EventLoop::isSingleThread() {
  return GetTid() == tid_;
}

void EventLoop::wakeup() {
  uint64_t one = 1;
  write(wake_, &one, sizeof one);
}

void EventLoop::handleRead() {
  uint64_t one = 1;
  read(wake_, &one, sizeof one);
}

EventLoop::~EventLoop() {
  close(wake_);
}

void EventLoop::Stop() {
  loopping_ = false;
  if (!isSingleThread()) {
    wakeup();
  }
}

void EventLoop::Update(Channel *p) {
  poller_->Update(p);
}

void EventLoop::RunInLoop(std::function<void()>func) {
  if (isSingleThread()) {
    func();
  }else {
    AddTask(std::move(func));
  }
}
void EventLoop::AddTask(std::function<void()>func) {
  {
    MutexLock lock(mutex_);
    tasks_.push_back(std::move(func));
  }
  if (!isSingleThread() || handing_) {
    wakeup();
  }
}

void EventLoop::Loop() {
  loopping_ = true;
  while(loopping_) {
    channels_.clear();
    poller_->Poll(&channels_, 1e3);
    for(size_t i = 0; i != channels_.size(); i++) {
      channels_[i]->Handle();
    }
    
    handing_ = true;
    std::vector<std::function<void()>>task;
    {
      MutexLock lock(mutex_);
      tasks_.swap(task);
    }
    for (size_t i = 0; i < task.size(); i++) {
      task[i]();
    }
    handing_ = false;
  }
}
