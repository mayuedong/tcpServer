#ifndef _SOCKET_H_
#define _SOCKET_H_

class Socket {
 public:
  Socket();
  ~Socket();
  int Fd();
  void Bind(struct sockaddr_in *addr);
  void Listen();
  int Accept(struct sockaddr_in *addr);
  void Shutdown();
  void NoDelay(bool on);
  void ReuseAddr(bool on);
  void KeepAlive(bool on);
 private:
  int fd_;
};

#endif//_SOCKET_H_
