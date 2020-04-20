#include "SLog.h"
#include "BuildConfig.h"
#include "core/Egc.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
namespace EgcCommon {
std::shared_ptr<spdlog::logger> SLog::s_ClientLogger;
std::shared_ptr<spdlog::logger> SLog::s_GrpcLogger;

#if defined(OS_WIN)

#define STRING_FMT_MAX_LENGHT 0x2000
void FmtV(std::string& out_str, const char* fmt, va_list ap) {
  std::string tmp(STRING_FMT_MAX_LENGHT, 0);
  _vsnprintf_s((char*)tmp.c_str(), STRING_FMT_MAX_LENGHT - 1,
               STRING_FMT_MAX_LENGHT, fmt, ap);
  out_str = tmp.c_str();
}

std::string Fmt(const char* szFmt, ...) {
  if (!szFmt) {
    return "";
  }

  std::string tmp;
  va_list ap;
  va_start(ap, szFmt);
  FmtV(tmp, szFmt, ap);
  va_end(ap);

  return tmp.c_str();
}

std::string GetNowTime(const char* split_time = "_") {
  SYSTEMTIME sys;
  GetLocalTime(&sys);
  return Fmt("%02d%s%02d%s%02d.%03d", sys.wHour, split_time, sys.wMinute,
             split_time, sys.wSecond, sys.wMilliseconds);
}

#endif
void SLog::Init(std::string grpclog_path) {
  // https://github.com/gabime/spdlog/wiki/3.-Custom-formatting for more
  spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e][%t][%l] %v%$");
  s_ClientLogger = spdlog::stdout_color_mt(EGC_CONSOLE_LOG_NAME);
  s_ClientLogger->set_level(spdlog::level::trace);
#if defined(OS_WIN)
  std::string logname = "\\" + GRPC_LOG_NAME + "_" + GetNowTime() + ".log";
#else
  std::string logname = "//" + GRPC_LOG_NAME + ".log";
#endif
  s_GrpcLogger = spdlog::rotating_logger_mt(
      EGC_FILE_LOG_NAME,
      grpclog_path + logname,
      1048576 * 5, 3);

  s_GrpcLogger->set_level(spdlog::level::trace);
}

}  // namespace EgcCommon
