#pragma once
#include <cstdint>
#include <iostream>

#ifndef H_CASE_STRING_BIGIN
#define H_CASE_STRING_BIGIN(state) switch (state) {
#define H_CASE_STRING(state) \
  case state:                \
    return #state;           \
    break;
#define H_CASE_STRING_END() \
  default:                  \
    return "Unknown";       \
    break;                  \
    }
#endif

#if defined(OS_WIN)
#if (__cplusplus >= 201103L && __cplusplus < 201402L)
namespace std {
// Note: despite not being even C++11 compliant, Visual Studio 2013 has their
// own implementation of std::make_unique. Define std::make_unique for pre-C++14
template <typename T, typename... Args>
inline unique_ptr<T> make_unique(Args&&... args) {
  return unique_ptr<T>(new T(forward<Args>(args)...));
}
}  // namespace std
#endif  // C++11 <= version < C++14
#endif  // WIN32
namespace EgcCommon {
//µ÷ÊÔÓÃ
std::uint64_t getHaomiaoAfterSeconds(uint32_t num_seconds);
//ºÁÃë
std::uint64_t getCurHaomiao();
std::uint64_t getCurWeimiao();
std::uint64_t genSuid(uint32_t uid, uint16_t terminaltype);
std::string Now();
uint64_t getSeqId();

}  // namespace EgcCommon
