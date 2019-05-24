#include "channel.h"
#include "eventLoop.h"
#include "poller.h"
#include <poll.h>
#include "eventLoop.h"
Channel::Channel(EventLoop *loop, int fd) 
: fd_(fd)
, mask_(0)
, fire_(0)
, state_(-1)
, loop_(loop){
}

void Channel::AddReadEvent() {
  mask_ |= POLLIN | POLLPRI;
  loop_->Update(this);
}
  void Channel::DelReadEvent() {
  mask_ &= ~(POLLIN | POLLPRI);
  loop_->Update(this);
}
  void Channel::AddWriteEvent() {
  mask_ |= POLLOUT;
  loop_->Update(this);
}
  void Channel::delWriteEvent() {
  mask_ &= ~POLLOUT;
  loop_->Update(this);
}
  void Channel::delAllEvent() {
  mask_ = 0;
  loop_->Update(this);
}

void Channel::Handle() {
  if ((fire_ & POLLHUP) && !(fire_ & POLLIN)) {
    if (close_) {
      close_();
    }
  }
  if (fire_ & (POLLERR | POLLNVAL)) {
    if (error_) {
      error_();
    }
  }
  if (fire_ & (POLLIN | POLLPRI | POLLRDHUP)) {
    if (read_) {
      read_();
    }
  }
  if (fire_ & POLLOUT) {
    if (write_) {
      write_();
    }
  }
}

