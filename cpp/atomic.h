#ifndef _ATOMIC_H_
#define _ATOMIC_H_

#include <stdint.h>

template<typename T>
class Atomic {
 public:
  Atomic()
    : value_(0) {
  }

  T Get() {
    return __sync_val_compare_and_swap(&value_, 0, 0);
  }

  T GetAndAdd(T x) {
    return __sync_fetch_and_add(&value_, x);
  }

  T AddAndGet(T x) {
    return GetAndAdd(x) + x;
  }

  T IncrementAndGet() {
    return AddAndGet(1);
  }

  T DecrementAndGet() {
    return AddAndGet(-1);
  }

  void Add(T x) {
    GetAndAdd(x);
  }

  void Increment() {
    IncrementAndGet();
  }

  void Decrement() {
    DecrementAndGet();
  }

  T GetAndSet(T newValue) {
    return __sync_lock_test_and_set(&value_, newValue);
  }

 private:
  volatile T value_;
};

#endif//_ATOMIC_H_
