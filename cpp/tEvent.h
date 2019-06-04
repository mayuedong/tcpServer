#ifndef _T_EVENT_H_
#define _T_EVENT_H_

#include "timeStamp.h"
#include "timerEnter.h"
#include <set>
#include <vector>
#include <memory>
#include <functional>

class Timer;
class EventLoop;
class Fevent;
class Tevent {
public:
  Tevent(EventLoop *loop);
  ~Tevent();
  TimerEnter AddTimeEvent(TimeStamp when, double interval, std::function<void()>func);
  void CancelEvent(TimerEnter enter);
private:
  void handle();
  void handleRead();
  bool insert(Timer *timer);
  void addTimeEvent(Timer *timer);
  void cancelEvent(TimerEnter enter);
  void reset(std::vector<std::pair<TimeStamp, Timer*>> &expEvent);
  void getExpEvent(std::vector<std::pair<TimeStamp, Timer*>> &expEvent, TimeStamp now);
  bool handing_;
  int wake_;
  EventLoop *loop_;
  std::unique_ptr<Fevent>wakeFevent_;
  std::set<std::pair<TimeStamp, Timer*>>tEvents_;
  std::set<std::pair<Timer*, int64_t>>dupEvents_;
  std::set<std::pair<Timer*, int64_t>>cancelEvents_;
};
#endif//_T_EVENT_H_
