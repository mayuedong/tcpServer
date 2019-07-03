#include "server.h"
#include "eventLoop.h"
#include "threadEventLoop.h"
#include "eventLoopPool.h"
#include "acceptor.h"
#include "connection.h"
#include <netinet/in.h>

Server::Server(EventLoop *loop, int port, int numThread)
: loop_(loop)
, nextId_(0)
, acceptor_(new Acceptor(loop_, port))
, pool_(new EventLoopPool(loop_, numThread))
//, connectionCallback_(defaultConnectionCallback)
//, messageCallback_(defaultMessageCallback)
  {
  acceptor_->SetFunc(std::bind(&Server::handle, this, std::placeholders::_1, std::placeholders::_2));
}

Server::~Server(){
  loop_->IsInLoop();  
  for (auto& it : connections_) {
    std::shared_ptr<Connection>ptr(it.second);
    it.second.reset();
    ptr->GetLoop()->RunInLoop(std::bind(&Connection::Destroy, ptr));
  }
}

void Server::Start(){
  pool_->Start();
  loop_->RunInLoop(std::bind(&Acceptor::Listen, acceptor_.get()));
}

void Server::handle(int sockfd, const struct sockaddr_in& peerAddr){
  loop_->IsInLoop();  
  EventLoop *loop = pool_->GetLoop();
  nextId_++;
  struct sockaddr_in addr;
  std::shared_ptr<Connection>ptr = std::make_shared<Connection>(loop, nextId_, sockfd, addr, peerAddr);
  /*
  ptr->setConnectionCallback(connectionCallback_);
  ptr->setMessageCallback(messageCallback_);
  ptr->setWriteCompleteCallback(writeCompleteCallback_);
  ptr->setCloseCallback(std::bind(&TcpServer::removeConnection, this, _1));
  loop->runInLoop(std::bind(&Connection::connectEstablished, ptr));
  */
  connections_[nextId_] = ptr;
}

void Server::delConnection(const std::shared_ptr<Connection> &ptr) {
  loop_->IsInLoop();  
  connections_.erase(ptr->GetId());
  EventLoop *loop = ptr->GetLoop();
  loop->AddTask(std::bind(&Connection::Destroy, ptr));
}
