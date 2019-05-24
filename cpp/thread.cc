#include "thread.h"
/*
#include <sys/syscall.h>

void CurrentThread::cacheTid() {
  if (0 == t_cachedTid) {
    t_cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));
  }
}
*/
Thread::Thread(std::function<void(void)>func)
: func_(func) {
}

class threadData {
public:
  threadData(pid_t *tid, std::function<void(void)>func)
  : tid_(tid)
  , func_(func){
  }

  pid_t *tid_;
  std::function<void(void)>func_;
};

void *run(void *data) {
  threadData *ptr = static_cast<threadData*>(data);
  *(ptr->tid_) = 99;
  ptr->func_();
  delete ptr;
  return NULL;
}

void Thread::Start() { 
  threadData *ptr = new threadData(&tid_, func_);
  if (pthread_create(&id_, NULL, run, ptr)) {
    delete ptr;
  }
}

void Thread::Join() {
  pthread_join(id_, NULL);
}
