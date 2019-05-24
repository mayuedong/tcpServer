#ifndef _EVENT_LOOP_H_
#define _EVENT_LOOP_H_

#include <memory>
#include <vector>

class Poller;
class Channel;
class EventLoop {
public:
  EventLoop(const EventLoop&) = delete;
  void operator=(const EventLoop&) = delete;
  EventLoop();
  ~EventLoop();
  void Update(Channel *p);
  void Loop();
  void Stop();
private:
  bool lopping_;
  std::unique_ptr<Poller>poller_;
  std::vector<Channel*>channels_;
};
#endif//_EVENT_LOOP_H_
