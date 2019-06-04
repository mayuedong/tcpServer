#include <sys/epoll.h>
#include <unistd.h>
#include <cstring>
#include "poller.h"
#include "fEvent.h"

Poller::Poller()
: epfd_(epoll_create1(EPOLL_CLOEXEC))
, events_(16) {
}

Poller::~Poller() {
  close(epfd_);
}

void Poller::Poll(std::vector<Fevent*> *fEvents, int timeout) {
	int count = epoll_wait(epfd_, events_.data(), events_.size(), timeout);
  for (int i = 0; i < count; i++) {
    Fevent *ptr = static_cast<Fevent*>(events_[i].data.ptr);
    ptr->SetFire(events_[i].events);
    fEvents->push_back(ptr);
  }
  if (static_cast<size_t>(count) == events_.size()) {
    events_.resize(2 * events_.size());
  }
}

void Poller::Remove(Fevent *p) {
  if (1 == p->GetState()) {
    update(p, EPOLL_CTL_DEL); 
  }
  p->SetState(2);
}

void Poller::Update(Fevent *p) {
  int state = p->GetState();
  if (-1 == state || 2 == state) {
    update(p, EPOLL_CTL_ADD); 
    p->SetState(1);
  }else if (1 == state){
    if (p->NoneEvent()) {
      update(p, EPOLL_CTL_DEL); 
      p->SetState(2);
    }else {
      update(p, EPOLL_CTL_MOD); 
    }
  }
}

void Poller::update(Fevent *p, int op) {
  struct epoll_event event;
  memset(&event, 0, sizeof(event));
  event.events = p->GetMask();
  event.data.ptr = p;
  epoll_ctl(epfd_, op, p->GetFd(), &event);
}
