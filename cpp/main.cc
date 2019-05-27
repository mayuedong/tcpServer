#include <cstdio>
#include <unistd.h>
#include <cstring>
#include <sys/timerfd.h>
#include "eventLoop.h"
#include "channel.h"
#include "thread.h"
#include "infoThread.h"
#include "threadPool.h"
#include "latch.h"

int fd;
EventLoop loop;

void func() {
  printf("receive\r\n");
  uint64_t one = 1;
  read(fd, &one, sizeof one);
  loop.Stop();
}
void func1(int n) {
  int old = n;
  int sum = 0;
  while(n) {
    sum += n;
    --n;
  }
  printf("n:%d sum:%d tid:%d\r\n", old, sum, GetTid());
  sleep(1);
}
int main() {
/*
  fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  struct itimerspec how;
  memset(&how,0,sizeof(how));
  how.it_value.tv_sec = 2;
  timerfd_settime(fd,0,&how,NULL);

  Channel ch(&loop, fd);
  ch.SetRead(func);
  ch.AddReadEvent();
  loop.Loop();
*/
/*
  Thread thread(func1);
  thread.Start();
  thread.Join();
*/

  ThreadPool pool;
  pool.Start(10);
  for (int i = 101; i < 201; i++) {
    pool.Push(std::bind(func1, i));
  }
  Latch latch(1);
  pool.Push(std::bind(&Latch::Done, &latch));
  printf("main wait begin\r\n");
  latch.Wait();
  printf("main wait end\r\n");
  pool.Stop();
  return 0;

}
