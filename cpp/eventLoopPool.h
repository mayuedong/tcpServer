#ifndef _EVENT_LOOP_POOL_
#define _EVENT_LOOP_POOL_

#include <vector>
#include <functional>
#include <memory>
class EventLoop;
class ThreadEventLoop;
class EventLoopPool {
public:
  EventLoopPool(EventLoop *loop, int numThread);
  void Start();
  EventLoop* GetLoop();
private:
  size_t index_;
  int numThread_;
  EventLoop *loop_;
  std::vector<std::unique_ptr<ThreadEventLoop>>pool_;
  std::vector<EventLoop*>loops_;
};
#endif//_EVENT_LOOP_POOL_
