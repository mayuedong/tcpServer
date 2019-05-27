#ifndef _EVENT_LOOP_H_
#define _EVENT_LOOP_H_

#include "infoThread.h"
#include "mutex.h"
#include <memory>
#include <vector>
#include <functional>

class Poller;
class Channel;
class EventLoop {
public:
  EventLoop();
  ~EventLoop();
  void Update(Channel *p);
  void Loop();
  void Stop();
  void AddTask(std::function<void()>func);
  void RunInLoop(std::function<void()>func);
private:
  bool isSingleThread();
  void wakeup();
  void handleRead();
  bool loopping_;
  bool handing_;
  int wake_;
  pid_t tid_;
  std::unique_ptr<Poller>poller_;
  std::vector<Channel*>channels_;

  Mutex mutex_;
  std::vector<std::function<void()>>tasks_;
  std::unique_ptr<Channel>wakeChannel_;
};
#endif//_EVENT_LOOP_H_
