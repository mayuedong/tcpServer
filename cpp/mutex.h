#ifndef _MUTEX_H_
#define _MUTEX_H_

#include <pthread.h>

class Mutex {
public:
  Mutex() {
    pthread_mutex_init(&mutex_, NULL);
  }

  ~Mutex() {
    pthread_mutex_destroy(&mutex_);
  }

  void lock() {
    pthread_mutex_lock(&mutex_);
  }

  void unlock() {
    pthread_mutex_unlock(&mutex_);
  }

  pthread_mutex_t* GetMutex() {
    return &mutex_;
  }

  pthread_mutex_t mutex_;
};

class MutexLock {
 public:
  MutexLock(const MutexLock&) = delete;
  void operator=(const MutexLock&) = delete;
  MutexLock(Mutex& mutex)
  : mutex_(mutex) {
    mutex_.lock();
  }

  ~MutexLock() {
    mutex_.unlock();
  }

 private:
  Mutex& mutex_;
};
#endif//_MUTEX_H_
