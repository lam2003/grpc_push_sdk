#pragma once

#include <stdint.h>
#include <string>
#include "Egc.h"
#include "PlatformDefine.h"

namespace EgcCommon {

class Timestamp {
 public:
  explicit Timestamp(int64_t iMicroSecondsSinceEpoch = 0);

  Timestamp(const Timestamp& rhs);

  static Timestamp Now();

  static std::string GetCurrentLocalTime();

  static double DiffInSeconds(const Timestamp& t1, const Timestamp& t2);

  static double DiffInMilliSedonds(const Timestamp& t1, const Timestamp& t2);

  static int64_t DiffInMicroSeconds(const Timestamp& t1, const Timestamp& t2);

  std::string ToCalenderTime();

  static Timestamp AddSeconds(Timestamp timestamp, double seconds);

  static Timestamp AddMilliseconds(Timestamp timestamp, double milliSeconds);

  int64_t GetMicroSecondsSineEpoch() const { return microSecondsSinceEpoch_; }

  bool operator<(const Timestamp& rhs) {
    return this->microSecondsSinceEpoch_ < rhs.GetMicroSecondsSineEpoch();
  }

  bool operator==(const Timestamp& rhs) {
    return this->microSecondsSinceEpoch_ == rhs.GetMicroSecondsSineEpoch();
  }

  bool operator<=(const Timestamp& rhs) {
    return this->microSecondsSinceEpoch_ <= rhs.GetMicroSecondsSineEpoch();
  }

  bool operator>(const Timestamp& rhs) {
    return this->microSecondsSinceEpoch_ > rhs.GetMicroSecondsSineEpoch();
  }

 private:
  int64_t microSecondsSinceEpoch_;
};

}  // namespace EgcCommon


