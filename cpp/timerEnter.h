#ifndef _TIMER_ENTER_H_
#define _TIMER_ENTER_H_

#include "timer.h"
class TimerEnter {
public:
  TimerEnter(Timer *timer, int64_t id) 
  : timer_(timer)
  , id_(id) {
  }
  int64_t GetId() {return id_;}
  Timer* GetTimer() {return timer_;}
private:
  Timer *timer_;
  int64_t id_;
};

#endif//_TIMER_ENTER_H_
