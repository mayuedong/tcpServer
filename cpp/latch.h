#ifndef _LATCH_H_
#define _LATCH_H_

#include "cond.h"

class Latch {
public:
  Latch(int count) 
  : count_(count) 
  , mutex_()
  , cond_(mutex_) {
  }

  void Wait() {
    MutexLock lock(mutex_);
    while (0 < count_) {
     cond_.Wait(); 
    }
  }

  void Done() {
    MutexLock lock(mutex_);
    --count_;
    if (0 == count_) {
      cond_.NotifyAll();
    }
  }

private:
  int count_;
  Mutex mutex_;
  Cond cond_; 
};
#endif//_LATCH_H_
