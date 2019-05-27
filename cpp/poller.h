#ifndef _POLLER_H_
#define _POLLER_H_
#include <vector>
class Channel;
struct epoll_event;
class Poller {
public:
  Poller();
  ~Poller();
  void Poll(std::vector<Channel*> *channels, int timeout);
  void Update(Channel *p);
  void Remove(Channel *p);
private:
  void update(Channel *p, int op);
  int epfd_;
  std::vector<struct epoll_event>events_;
};
#endif//_POLLER_H_
