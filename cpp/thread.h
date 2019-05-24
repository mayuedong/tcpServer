#ifndef _THREAD_H_
#define _THREAD_H_

#include <functional>
#include <pthread.h>
class Thread {
public:
  Thread(const Thread&) = delete;
  void operator=(const Thread&) = delete;
  Thread(std::function<void(void)>func);
  void Start();
  void Join();
  pid_t GetTid() {return tid_;}
private:
  pthread_t id_;
  pid_t tid_;
  std::function<void(void)>func_;
};
#endif//_THREAD_H_
