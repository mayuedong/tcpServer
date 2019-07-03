#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_
#include <memory>
#include <functional>
#include <vector>
#include <deque>
#include "mutex.h"
#include "cond.h"
class Thread;
class ThreadPool {
public:
  ThreadPool(int numThread);
  ~ThreadPool();
  void Start();
  void Stop();
  void Push(std::function<void(void)>func);
private:
  void run();
  std::function<void(void)> getTask();
  bool running_;
  int numThread_;
  Mutex mutex_;
  Cond full_;
  Cond empty_;
  std::deque<std::function<void(void)>>tasks_;
  std::vector<std::unique_ptr<Thread>>threads_;
};
#endif//_THREAD_POOL_H_
