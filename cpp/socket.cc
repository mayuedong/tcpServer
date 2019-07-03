#include "socket.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

Socket::Socket() {
  fd_ = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
  if (0 > fd_) {
    abort();
  }
}

Socket::~Socket() {
  close(fd_);
}

void Socket::Bind(struct sockaddr_in *addr) {
  bind(fd_, (struct sockaddr*)addr, sizeof(struct sockaddr_in));
}

void Socket::Listen() {
  listen(fd_, SOMAXCONN);
}

int Socket::Fd() {
  return fd_;
}

int Socket::Accept(struct sockaddr_in *addr) {
  memset(addr, 0, sizeof(struct sockaddr_in));
  socklen_t addrlen = sizeof(struct sockaddr_in);
  int connfd = ::accept4(fd_, (struct sockaddr*)addr, &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
  return connfd;
}

void Socket::Shutdown() {
  shutdown(fd_, SHUT_WR);
}

void Socket::ReuseAddr(bool on){
  int optval = on ? 1 : 0;
  setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
}

void Socket::KeepAlive(bool on){
  int optval = on ? 1 : 0;
  setsockopt(fd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof optval);
}

void Socket::NoDelay(bool on){
  //int optval = on ? 1 : 0;
  //setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof optval);
}
