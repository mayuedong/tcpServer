#ifndef _POLLER_H_
#define _POLLER_H_
#include <vector>
class Fevent;
struct epoll_event;
class Poller {
public:
  Poller();
  ~Poller();
  void Poll(std::vector<Fevent*> *channels, int timeout);
  void Update(Fevent *p);
  void Remove(Fevent *p);
private:
  void update(Fevent *p, int op);
  int epfd_;
  std::vector<struct epoll_event>events_;
};
#endif//_POLLER_H_
