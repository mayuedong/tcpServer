#ifndef _COND_H_
#define _COND_H_

#include "mutex.h"

class Cond {
public:
   Cond(Mutex& mutex)
   :  mutex_(mutex) {
    pthread_cond_init(&cond_, NULL);
  }

  ~Cond(){
    pthread_cond_destroy(&cond_);
  }

  void Wait() {
    pthread_cond_wait(&cond_, mutex_.GetMutex());
  }

  void WaitSec(double sec) {
    struct timespec abstime;
    clock_gettime(CLOCK_REALTIME, &abstime);

    const int64_t rate = 1e9;
    int64_t nasec = static_cast<int64_t>(sec * rate);
    abstime.tv_sec += static_cast<time_t>((abstime.tv_nsec + nasec) / rate);
    abstime.tv_nsec = static_cast<long>((abstime.tv_nsec + nasec) % rate);

    MutexLock lock(mutex_);
    pthread_cond_timedwait(&cond_, mutex_.GetMutex(), &abstime);
  }


  void Notify() {
    pthread_cond_signal(&cond_);
  }

  void NotifyAll() {
    pthread_cond_broadcast(&cond_);
  }

 private:
  Mutex& mutex_;
  pthread_cond_t cond_;
};
#endif//_COND_H_
