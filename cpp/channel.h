#ifndef _CHANNEL_H_
#define _CHANNEL_H_

#include <functional>
class EventLoop;
class Channel {
public:
  Channel(const Channel&) = delete;
  void operator=(const Channel&) = delete;
  Channel(EventLoop *loop, int fd);
  void Handle();

  void SetFire(int v) {fire_ = v;}
  void SetState(int v) {state_ = v;}

  void SetRead(std::function<void(void)>v) {read_ = std::move(v);}
  void SetWrite(std::function<void(void)>v) {write_ = std::move(v);}
  void SetError(std::function<void(void)>v) {error_ = std::move(v);}
  void SetClose(std::function<void(void)>v) {close_ = std::move(v);}

  void AddReadEvent();
  void DelReadEvent();
  void AddWriteEvent();
  void delWriteEvent();
  void delAllEvent();

  int GetFd() {return fd_;}
  int GetMask() {return mask_;}
  int GetState() {return state_;}

  bool NoneEvent() {return 0 == mask_;}
private:
  int fd_;
  int mask_;
  int fire_;
  int state_;
  EventLoop *loop_;
  std::function<void(void)>write_;
  std::function<void(void)>read_;
  std::function<void(void)>error_;
  std::function<void(void)>close_;
};
#endif//_CHANNEL_H_
