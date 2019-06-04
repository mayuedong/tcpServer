#ifndef _TIME_STAMP_H_
#define _TIME_STAMP_H_

#include <string>
class TimeStamp {
public:
  TimeStamp() {us_ = 0;}
  TimeStamp(int64_t us) {us_ = us;}
  void Swap(TimeStamp& r) {std::swap(us_, r.us_);}
  std::string String() const;
  std::string Format() const;
  static TimeStamp Now();
  int64_t GetUs() {return us_;}
  time_t UsecToSec() {return static_cast<time_t>(us_ / 1e6);}
  static TimeStamp AddUsec(time_t t, int us) {return TimeStamp(static_cast<int64_t>(t) * 1e6 + us);}
 private:
  int64_t us_;
};

inline bool operator<(TimeStamp lhs, TimeStamp rhs) {
  return lhs.GetUs() < rhs.GetUs();
}

inline bool operator<=(TimeStamp lhs, TimeStamp rhs) {
  return lhs.GetUs() <= rhs.GetUs();
}

inline bool operator>(TimeStamp lhs, TimeStamp rhs) {
  return lhs.GetUs() > rhs.GetUs();
}

inline bool operator>=(TimeStamp lhs, TimeStamp rhs) {
  return lhs.GetUs() >= rhs.GetUs();
}

inline bool operator==(TimeStamp lhs, TimeStamp rhs) {
  return lhs.GetUs() == rhs.GetUs();
}

inline double SubSec(TimeStamp high, TimeStamp low) {
  int64_t us = high.GetUs() - low.GetUs();
  return static_cast<double>(us) / 1e6;
}

inline TimeStamp AddSec(TimeStamp timestamp, double sec) {
  int64_t us = static_cast<int64_t>(sec * 1e6);
  return TimeStamp(timestamp.GetUs() + us);
}
#endif//_TIME_STAMP_H_
