#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "BuildConfig.h"
#include "EgcUtils.h"
#ifdef OS_WIN
#include <windows.h>
#endif

#include <chrono>
#include <cstdint>
#include "common/TimeStamp.h"
namespace EgcCommon {

std::uint64_t getHaomiaoAfterSeconds(uint32_t num_seconds) {
  auto after =
      std::chrono::system_clock::now() + std::chrono::seconds(num_seconds);
  auto dura = after.time_since_epoch();
  auto millis =
      std::chrono::duration_cast<std::chrono::milliseconds>(dura).count();
  return millis;
}
//毫秒
std::uint64_t getCurHaomiao() {
  std::chrono::time_point<std::chrono::system_clock> now =
      std::chrono::system_clock::now();
  auto duration = now.time_since_epoch();
  auto millis =
      std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
  return millis;
}
//微妙
std::uint64_t getCurWeimiao() {
  std::chrono::time_point<std::chrono::system_clock> now =
      std::chrono::system_clock::now();
  auto duration = now.time_since_epoch();
  auto micro =
      std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
  return micro;
}

uint64_t genSuid(uint32_t uid, uint16_t terminaltype) {
  uint64_t suid = 0;
  uint64_t type = uint64_t(terminaltype);
  suid |= type << (32 + 8);
  suid |= uint64_t(uid) & 0x00000000FFFFFFFF;
  return suid;
}

std::string Now() { return EgcCommon::Timestamp::Now().ToCalenderTime(); }

//非线程安全
uint64_t getSeqId() {
  static uint64_t SEQID = 0;
  return SEQID++;
}
}  // namespace EgcCommon
