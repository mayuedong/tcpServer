#include "connection.h"
Connection::Connection(EventLoop* loop, int id, int sockfd, const struct sockaddr_in& localAddr, const struct sockaddr_in& peerAddr)
  : loop_(loop)
  , id_(id)
/*    state_(kConnecting),
    reading_(true),
    socket_(new Socket(sockfd)),
    channel_(new Channel(loop, sockfd)),
    localAddr_(localAddr),
    peerAddr_(peerAddr),
    highWaterMark_(64*1024*1024)
*/
{
  /*
  channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this, _1)); channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
  channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
  channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));
  socket_->setKeepAlive(true);
*/
}

void Connection::Destroy() {

}
