#include "threadPool.h"
#include "thread.h"
ThreadPool::ThreadPool(int numThread) 
: running_(false)
, numThread_(numThread)
, mutex_()
, full_(mutex_)
, empty_(mutex_){
}

ThreadPool::~ThreadPool() {
  if (running_) {
    Stop();
  }
}

void ThreadPool::Stop() {
  {
    MutexLock lock(mutex_);
    running_ = false;
    full_.NotifyAll();
  }
  for (size_t i = 0; i != threads_.size(); i++) {
    threads_[i]->Join();
  }
}

void ThreadPool::Start() {
  running_ = true;
  threads_.reserve(numThread_);
  for (int i = 0; i < numThread_; i++) {
    threads_.emplace_back(new Thread(std::bind(&ThreadPool::run, this)));
    threads_[i]->Start();
  }
}

void ThreadPool::run() {
  while(running_) {
    std::function<void(void)> task(getTask());
    if (task) {
     task(); 
    }
  }
}

std::function<void(void)> ThreadPool::getTask() {
  MutexLock lock(mutex_);
  while(tasks_.empty() && running_) {
    full_.Wait();
  }
  if (tasks_.empty()) {
    return NULL;
  }
  std::function<void(void)>task(tasks_.front());
  tasks_.pop_front();
  if (tasks_.max_size() > tasks_.size()) {
    empty_.Notify();
  }
  return task;
}

void ThreadPool::Push(std::function<void(void)>func) {
  MutexLock lock(mutex_);
  while(tasks_.max_size() == tasks_.size()) {
    empty_.Wait();
  }
  tasks_.push_back(std::move(func));
  full_.Notify();
}
