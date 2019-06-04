#include "fEvent.h"
#include "eventLoop.h"
#include "poller.h"
#include <poll.h>
#include "eventLoop.h"
Fevent::Fevent(EventLoop *loop, int fd) 
: fd_(fd)
, mask_(0)
, fire_(0)
, state_(-1)
, loop_(loop){
}

void Fevent::AddReadEvent() {
  mask_ |= POLLIN | POLLPRI;
  loop_->Update(this);
}
  void Fevent::DelReadEvent() {
  mask_ &= ~(POLLIN | POLLPRI);
  loop_->Update(this);
}
  void Fevent::AddWriteEvent() {
  mask_ |= POLLOUT;
  loop_->Update(this);
}
  void Fevent::DelWriteEvent() {
  mask_ &= ~POLLOUT;
  loop_->Update(this);
}
  void Fevent::DelAllEvent() {
  mask_ = 0;
  loop_->Update(this);
}
  void Fevent::Remove() {
  loop_->Remove(this);
}

void Fevent::Handle() {
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

