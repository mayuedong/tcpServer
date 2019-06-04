#include "tEvent.h"
#include "fEvent.h"
#include "eventLoop.h"
#include "timer.h"
#include <sys/timerfd.h>
#include <unistd.h>
#include <cstring>

struct timespec timeDifference(TimeStamp when) {
  int64_t us = when.GetUs() - TimeStamp::Now().GetUs();
  struct timespec ts;
  ts.tv_sec = static_cast<time_t>(us / 1e6);
  ts.tv_nsec = static_cast<long>((us % (int64_t)1e6) * 1e3);
  return ts;
}

void resetWakeTime(int fd, TimeStamp when) {
  struct itimerspec newValue, oldValue;
  memset(&newValue, 0, sizeof newValue);
  memset(&oldValue, 0, sizeof oldValue);
  newValue.it_value = timeDifference(when);
  timerfd_settime(fd, 0, &newValue, &oldValue);
}

Tevent::Tevent(EventLoop *loop)
: handing_(false)
, wake_(timerfd_create(CLOCK_MONOTONIC,TFD_NONBLOCK | TFD_CLOEXEC))
, loop_(loop)
, wakeFevent_(new Fevent(loop_, wake_)){
  wakeFevent_->SetRead(std::bind(&Tevent::handle, this));
  wakeFevent_->AddReadEvent();
}
void Tevent::handleRead() {
  uint64_t one = 1;
  read(wake_, &one, sizeof one);
}

Tevent::~Tevent() {
  wakeFevent_->DelAllEvent();
  wakeFevent_->Remove();
  close(wake_);
  std::set<std::pair<TimeStamp, Timer*>>::iterator it = tEvents_.begin();
  for (; tEvents_.end() != it; ++it) {
    delete it->second;
  }
}

TimerEnter Tevent::AddTimeEvent(TimeStamp when, double interval, std::function<void()>func) {
  Timer *timer = new Timer(when, interval, std::move(func));
  loop_->RunInLoop(std::bind(&Tevent::addTimeEvent, this, timer));
  return TimerEnter(timer, timer->GetId());
}

void Tevent::addTimeEvent(Timer *timer) {
  loop_->IsInLoop();
  if (insert(timer)) {
    resetWakeTime(wake_, timer->GetWhen());
  }
}

void Tevent::CancelEvent(TimerEnter enter) {
  loop_->RunInLoop(std::bind(&Tevent::cancelEvent, this, enter));
}

void Tevent::cancelEvent(TimerEnter enter) {
  loop_->IsInLoop();
  std::pair<Timer*, int64_t>sentry(enter.GetTimer(), enter.GetId());
  std::set<std::pair<Timer*, int64_t>>::iterator it = dupEvents_.find(sentry);
  if (it != dupEvents_.end()) {
    tEvents_.erase(std::pair<TimeStamp, Timer*>(it->first->GetWhen(), it->first));
    delete it->first;
    dupEvents_.erase(it);
  } else if (handing_) {
    cancelEvents_.insert(sentry);
  }
}

bool Tevent::insert(Timer *timer) {
  TimeStamp when(timer->GetWhen());
  tEvents_.insert(std::pair<TimeStamp, Timer*>(when, timer));
  dupEvents_.insert(std::pair<Timer*, int64_t>(timer, timer->GetId()));
  std::set<std::pair<TimeStamp, Timer*>>::iterator it = tEvents_.begin();
  if (timer == it->second) {
    return true;
  }
  return false;
}

void Tevent::getExpEvent(std::vector<std::pair<TimeStamp, Timer*>> &expEvent, TimeStamp now) {
  std::pair<TimeStamp, Timer*>sentry(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
  std::set<std::pair<TimeStamp, Timer*>>::iterator end = tEvents_.lower_bound(sentry);
  std::copy(tEvents_.begin(), end, back_inserter(expEvent));
  tEvents_.erase(tEvents_.begin(), end);
  std::vector<std::pair<TimeStamp, Timer*>>::iterator it = expEvent.begin();
  for (; it != expEvent.end(); ++it) {
    std::pair<Timer*, int64_t>delEntry(it->second, it->second->GetId());
    dupEvents_.erase(delEntry);
  }
}

void Tevent::reset(std::vector<std::pair<TimeStamp, Timer*>> &expEvent) {
  std::vector<std::pair<TimeStamp, Timer*>>::iterator it = expEvent.begin();
  for (; it != expEvent.end(); ++it) {
    std::pair<Timer*, int64_t>sentry(it->second, it->second->GetId());
    if (it->second->Repeat() && cancelEvents_.end() == cancelEvents_.find(sentry)) {
      it->second->Reset(TimeStamp::Now());
    }else {
      delete it->second;
    }
  }
  if (!tEvents_.empty()) {
    resetWakeTime(wake_, tEvents_.begin()->second->GetWhen());
  }
}

void Tevent::handle() {
  loop_->IsInLoop();
  handleRead();
  std::vector<std::pair<TimeStamp, Timer*>>expEvent;
  getExpEvent(expEvent, TimeStamp::Now()); 
  handing_ = true;
  cancelEvents_.clear();
  for (size_t i = 0; i != expEvent.size(); ++i) {
    expEvent[i].second->Run();
  } 
  handing_ = false;
  reset(expEvent);
}
