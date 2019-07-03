#ifndef _ACCEPTOR_H_
#define _ACCEPTOR_H_

#include <functional>
#include "fEvent.h"
#include "socket.h"

class EventLoop;

class Acceptor {
public:
  Acceptor(EventLoop* loop, int port);
  ~Acceptor();
  void SetFunc(const std::function<void (int fd, const struct sockaddr_in&)>&func){func_ = func;}
  bool IsListen() const { return listenning_; }
  void Listen();

private:
  void handle();
  bool listenning_;
  int idleFd_;
  EventLoop* loop_;
  Socket socket_;
  Fevent event_;
  std::function<void (int fd, const struct sockaddr_in&)>func_;
};

#endif//_ACCEPTOR_H_
