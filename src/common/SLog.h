#pragma once

//在  spdlog.h   之前定义，才有效
#ifndef SPDLOG_TRACE_ON
#define SPDLOG_TRACE_ON
#endif
#include "BuildConfig.h"
#include "spdlog/spdlog.h"
// Support for logging for custom types
#include <memory>
#include "spdlog/fmt/ostr.h"
namespace EgcCommon {
class SLog {
 public:
  static void Init(std::string grpclog_path);
  inline static std::shared_ptr<spdlog::logger>& GetClientLogger() {
    return s_ClientLogger;
  }
  inline static std::shared_ptr<spdlog::logger>& GetGrpcLogger() {
    return s_GrpcLogger;
  }

 private:
  static std::shared_ptr<spdlog::logger> s_ClientLogger;
  static std::shared_ptr<spdlog::logger> s_GrpcLogger;
};

}  // namespace EgcCommon

#if defined(OS_WIN)
#define __FILENAME__ \
  (strrchr(__FILE__, '\\') ? (strrchr(__FILE__, '\\') + 1) : __FILE__)
#else
#define __FILENAME__ \
  (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1) : __FILE__)
#endif

//定义一个在日志后添加 文件名 函数名 行号 的宏定义
#ifndef suffix
#define suffix(msg)                     \
  std::string(msg)                      \
      .append("  <")                    \
      .append(__FILENAME__)             \
      .append("> <")                    \
      .append(__func__)                 \
      .append("> <")                    \
      .append(std::to_string(__LINE__)) \
      .append(">")                      \
      .c_str()
#endif


#define SLOGI(msg, ...) \
  ::EgcCommon::SLog::GetClientLogger()->info(suffix(msg), ##__VA_ARGS__)

#define SLOGV(msg, ...) \
  ::EgcCommon::SLog::GetClientLogger()->trace(suffix(msg), ##__VA_ARGS__)

#define SLOGW(msg, ...)                                                       \
  do {                                                                        \
    if (::EgcCommon::SLog::GetClientLogger())                                 \
      ::EgcCommon::SLog::GetClientLogger()->warn(suffix(msg), ##__VA_ARGS__); \
  } while (0)

#define SLOGE(msg, ...) \
  ::EgcCommon::SLog::GetClientLogger()->error(suffix(msg), ##__VA_ARGS__)

#define SLOGF(msg, ...) \
  ::EgcCommon::SLog::GetClientLogger()->critical(suffix(msg), ##__VA_ARGS__)


#define GLOGV(...)                                            \
  do {                                                        \
    if (::EgcCommon::SLog::GetGrpcLogger())                   \
      ::EgcCommon::SLog::GetGrpcLogger()->trace(__VA_ARGS__); \
  } while (0)