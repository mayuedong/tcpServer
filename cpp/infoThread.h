#ifndef _INFO_THREAD_H_
#define _INFO_THREAD_H_

extern __thread int __tid;
void getTid();
inline int GetTid() {
  if (0 == __tid) {
    getTid();
  }
  return __tid;
}
#endif//_INFO_THREAD_H_
