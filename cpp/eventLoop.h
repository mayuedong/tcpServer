#ifndef _EVENT_LOOP_H_
#define _EVENT_LOOP_H_

#include "infoThread.h"
#include "mutex.h"
#include "timerEnter.h"
#include <memory>
#include <vector>
#include <functional>

class Poller;
class Fevent;
class Tevent;
class EventLoop {
public:
  EventLoop();
  ~EventLoop();
  void Update(Fevent *p);
  void Remove(Fevent *p);
  void Loop();
  void Stop();
  void AddTask(std::function<void()>func);
  void RunInLoop(std::function<void()>func);
  void IsInLoop();
  TimerEnter RunAt(TimeStamp when, std::function<void()>func);
  TimerEnter RunAfter(double delay, std::function<void()>func);
  TimerEnter RunEvery(double interval, std::function<void()>func);
  void CancelTimeEvent(TimerEnter enter);
private:
  bool isSingleThread();
  void wakeup();
  void handleRead();
  bool loopping_;
  bool handing_;
  int wake_;
  pid_t tid_;
  std::unique_ptr<Poller>poller_;
  std::unique_ptr<Tevent>tEvent_;
  std::vector<Fevent*>fEvents_;

  Mutex mutex_;
  std::vector<std::function<void()>>tasks_;
  std::unique_ptr<Fevent>wakeFevent_;
};
#endif//_EVENT_LOOP_H_
