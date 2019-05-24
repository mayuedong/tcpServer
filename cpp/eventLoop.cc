#include "eventLoop.h"
#include "channel.h"
#include "poller.h"

EventLoop::EventLoop()
: lopping_(false)
, poller_(new Poller()) {

}

EventLoop::~EventLoop() {

}

void EventLoop::Stop() {
  lopping_ = false;
}
void EventLoop::Update(Channel *p) {
  poller_->Update(p);
}

void EventLoop::Loop() {
  lopping_ = true;
  while(lopping_) {
    channels_.clear();
    poller_->Poll(&channels_, 1e3);
    for(size_t i = 0; i != channels_.size(); i++) {
      channels_[i]->Handle();
    }
  }
}
