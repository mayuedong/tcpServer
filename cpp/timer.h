#ifndef _TIMER_H_
#define _TIMER_H_

#include "timeStamp.h"
#include "atomic.h"
#include <functional>
class Timer{
public:
  Timer(TimeStamp when, double interval, std::function<void()>func)
  : when_(when)
  , id_(g_counter_.IncrementAndGet())
  , interval_(interval)
  , func_(std::move(func)){
  }
  void Run() {func_();}
  int64_t GetId() {return id_;}
  TimeStamp GetWhen() {return when_;}
  bool Repeat() {return 0 < interval_;}
  void Reset(TimeStamp now) {when_ = AddSec(now, interval_);}
private:
  TimeStamp when_;
  int64_t id_;
  double interval_;
  std::function<void()>func_;
  static Atomic<int64_t>g_counter_; 
};
#endif//_TIMER_H_
