#include <cstdio>
#include <vector>
#include <ctime>
#include <unistd.h>
#include <cstring>
#include <sys/timerfd.h>
#include "eventLoop.h"
#include "fEvent.h"
#include "thread.h"
#include "infoThread.h"
#include "threadPool.h"
#include "timerEnter.h"
#include "latch.h"
#include "threadEventLoop.h"
#include "eventLoopPool.h"

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
void func2(int n) {
  printf("handing %d time:%ld\r\n", n, time(NULL));
}
void func3() {
  loop.RunAfter(5, std::bind(func2, 5));
  TimerEnter t3 = loop.RunEvery(3, std::bind(func2, 3));
  sleep(30);
  printf("begin cancel %ld\r\n",time(NULL));
  loop.CancelTimeEvent(t3);
  sleep(10);
  loop.Stop();
}
void func4(EventLoop *loop) {
  loop->RunAfter(5, std::bind(func2, 5));
  TimerEnter t3 = loop->RunEvery(3, std::bind(func2, 3));
  sleep(30);
  printf("begin cancel %ld\r\n",time(NULL));
  loop->CancelTimeEvent(t3);
  sleep(10);
  loop->Stop();
}
int main() {
//eventloop file event
/*
  fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  struct itimerspec how;
  memset(&how,0,sizeof(how));
  how.it_value.tv_sec = 2;
  timerfd_settime(fd,0,&how,NULL);

  Fevent ch(&loop, fd);
  ch.SetRead(func);
  ch.AddReadEvent();
  loop.Loop();
*/
//thread
/*
  Thread thread(func1);
  thread.Start();
  thread.Join();
*/
//thread pool
/*
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
*/
// eventloop time event
/*
  printf("begin time:%ld\r\n", time(NULL));
  Thread thread(func3);
  thread.Start();
  loop.Loop();
  thread.Join();
*/
// thread eventloop
/*
  ThreadEventLoop threadEvent;
  EventLoop *ptr = threadEvent.Start();
  func4(ptr);
*/
// eventloop pool
/*
  EventLoopPool pool(&loop, 5);
  Thread thread(std::bind(&EventLoopPool::Start, &pool, 5));
  thread.Start();
  sleep(3);
  //pool.Start(5);
  for (int i = 0; i < 10; i++) {
    printf("i:%d p:%p\r\n", i, pool.GetLoop());
  }
*/
  return 0;

}
