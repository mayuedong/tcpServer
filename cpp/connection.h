#ifndef _CONNECTION_H_
#define _CONNECTION_H_
 
class EventLoop;
class Connection {
public:
  Connection(EventLoop *loop, int id, int sockfd, const struct sockaddr_in &addr, const struct sockaddr_in &peerAddr);
  EventLoop* GetLoop(){return loop_;}
  int GetId(){return id_;}
  void Destroy();
private:
  EventLoop* loop_;
  int id_;
};

#endif//_CONNECTION_H_
