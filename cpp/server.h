#ifndef _SERVER_H_
#define _SERVER_H_

#include <map>
#include <memory>
class Acceptor;
class Connection;
class EventLoop;
class EventLoopPool;
class Server {
public:
  Server(EventLoop *loop, int port, int numThread);
  ~Server();
  void Start();
private:
  void delConnection(const std::shared_ptr<Connection> &ptr);
  void handle(int sockfd, const struct sockaddr_in& peerAddr);
  EventLoop *loop_;
  int nextId_;
  std::unique_ptr<Acceptor>acceptor_;
  std::shared_ptr<EventLoopPool>pool_;
  std::map<int, std::shared_ptr<Connection>>connections_;
};
#endif//_SERVER_H_
