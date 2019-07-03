#include "eventLoop.h"
#include "fEvent.h"
#include "tEvent.h"
#include "poller.h"
#include <sys/eventfd.h>
#include <unistd.h>
#include <stdlib.h>

EventLoop::EventLoop()
: loopping_(false)
, handing_(false)
, wake_(eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC))
, tid_(GetTid())
, poller_(new Poller()) 
, tEvent_(new Tevent(this))
, wakeFevent_(new Fevent(this, wake_)) {
  wakeFevent_->SetRead(std::bind(&EventLoop::handleRead, this));
  wakeFevent_->AddReadEvent();
}

EventLoop::~EventLoop() {
  wakeFevent_->DelAllEvent();
  wakeFevent_->Remove();
  close(wake_);
}

bool EventLoop::isSingleThread() {
  return GetTid() == tid_;
}

void EventLoop::IsInLoop() {
  if (!isSingleThread()) {
    abort();
  }
}

void EventLoop::wakeup() {
  uint64_t one = 1;
  write(wake_, &one, sizeof one);
}

void EventLoop::handleRead() {
  uint64_t one = 1;
  read(wake_, &one, sizeof one);
}

void EventLoop::Stop() {
  loopping_ = false;
  if (!isSingleThread()) {
    wakeup();
  }
}

void EventLoop::Update(Fevent *p) {
  IsInLoop();
  poller_->Update(p);
}
void EventLoop::Remove(Fevent *p) {
  IsInLoop();
  poller_->Remove(p);
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
  IsInLoop();
  loopping_ = true;
  while(loopping_) {
    fEvents_.clear();
    poller_->Poll(&fEvents_, 1e3);
    for(size_t i = 0; i != fEvents_.size(); i++) {
      fEvents_[i]->Handle();
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

TimerEnter EventLoop::RunAt(TimeStamp when, std::function<void()>func) {
  return tEvent_->AddTimeEvent(when, 0, std::move(func));
}

TimerEnter EventLoop::RunAfter(double delay, std::function<void()>func) {
  TimeStamp when(AddSec(TimeStamp::Now(), delay));
  return tEvent_->AddTimeEvent(when, 0, std::move(func));
}

TimerEnter EventLoop::RunEvery(double interval, std::function<void()>func) {
  TimeStamp when(AddSec(TimeStamp::Now(), interval));
  return tEvent_->AddTimeEvent(when, interval, std::move(func));
}

void EventLoop::CancelTimeEvent(TimerEnter enter) {
  tEvent_->CancelEvent(enter);
}
