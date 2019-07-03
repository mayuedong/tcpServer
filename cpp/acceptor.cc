#include "acceptor.h"
#include "eventLoop.h"
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <netinet/in.h>

Acceptor::Acceptor(EventLoop* loop, int port)
: listenning_(false)
, idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC))
, loop_(loop)
, socket_()
, event_(loop, socket_.Fd()) {
  socket_.ReuseAddr(true);
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof addr);
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(port);
  socket_.Bind(&addr);
  event_.SetRead(std::bind(&Acceptor::handle, this));
}

Acceptor::~Acceptor(){
  event_.DelAllEvent();
  event_.Remove();
  close(idleFd_);
}

void Acceptor::Listen() {
  loop_->IsInLoop();
  listenning_ = true;
  socket_.Listen();
  event_.AddReadEvent();
}

void Acceptor::handle() {
  loop_->IsInLoop();
  struct sockaddr_in peerAddr;
  int connfd = socket_.Accept(&peerAddr);
  if (0 > connfd && errno == EMFILE) {
    close(idleFd_);
    idleFd_ = accept(socket_.Fd(), NULL, NULL);
    close(idleFd_);
    idleFd_ = open("/dev/null", O_RDONLY | O_CLOEXEC);
    return;
  }
  if (0 <= connfd) {
    if (func_){
      func_(connfd, peerAddr);
    }else{
      close(connfd);
    }
  }
}
