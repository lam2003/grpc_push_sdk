#include "EgcGrpcLog.h"
#include <grpc/support/log.h>
#include <sys/timeb.h>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include "BuildConfig.h"
#include "SLog.h"

namespace EgcCommon {
namespace EgcGrpcLog {

#if defined(OS_WIN)
static LARGE_INTEGER g_start_time;
static double g_time_scale;
#endif

bool g_Verbose = false;
std::ofstream g_logfile;
struct tm g_localTime;
#if defined(OS_WIN)
#define localtime_r(_Time, _Tm) localtime_s(_Tm, _Time)
#endif

/** The clocks we support. */
typedef enum {
  /** Monotonic clock. Epoch undefined. Always moves forwards. */
  EDU_CLOCK_MONOTONIC = 0,
  /** Realtime clock. May jump forwards or backwards. Settable by
         the system administrator. Has its epoch at 0:00:00 UTC 1 Jan 1970. */
  EDU_CLOCK_REALTIME,
  /** CPU cycle time obtained by rdtsc instruction on x86 platforms. Epoch
         undefined. Degrades to GPR_CLOCK_REALTIME on other platforms. */
  EDU_CLOCK_PRECISE,
  /** Unmeasurable clock type: no base, created by taking the difference
         between two times */
  EDU_TIMESPAN
} edu_clock_type;
/** Analogous to struct timespec. On some machines, absolute times may be in
 * local time. */
typedef struct edu_timespec {
  int64_t tv_sec;
  int32_t tv_nsec;
  /** Against which clock was this time measured? (or GPR_TIMESPAN if
          this is a relative time measure) */
  edu_clock_type clock_type;
} edu_timespec;
#if defined(OS_WIN)

static edu_timespec now_impl(edu_clock_type clock) {
  edu_timespec now_tv;
  LONGLONG diff;
  struct _timeb now_tb;
  LARGE_INTEGER timestamp;
  double now_dbl;
  now_tv.clock_type = clock;
  switch (clock) {
    case EDU_CLOCK_REALTIME:
      _ftime_s(&now_tb);
      now_tv.tv_sec = (int64_t)now_tb.time;
      now_tv.tv_nsec = now_tb.millitm * 1000000;
      break;
    case EDU_CLOCK_MONOTONIC:
    case EDU_CLOCK_PRECISE:
      QueryPerformanceCounter(&timestamp);
      diff = timestamp.QuadPart - g_start_time.QuadPart;
      now_dbl = (double)diff * g_time_scale;
      now_tv.tv_sec = (int64_t)now_dbl;
      now_tv.tv_nsec = (int32_t)((now_dbl - (double)now_tv.tv_sec) * 1e9);
      break;
    case EDU_TIMESPAN:
      abort();
      break;
  }
  return now_tv;
}

#endif
const tm* getLocalTime() {
  auto now = std::chrono::system_clock::now();
  auto in_time_t = std::chrono::system_clock::to_time_t(now);
  localtime_r(&in_time_t, &g_localTime);
  return &g_localTime;
}
std::ostream& operator<<(std::ostream& stream, const tm* tm) {
  return stream << 1900 + tm->tm_year << '-' << std::setfill('0')
                << std::setw(2) << tm->tm_mon + 1 << '-' << std::setfill('0')
                << std::setw(2) << tm->tm_mday << ' ' << std::setfill('0')
                << std::setw(2) << tm->tm_hour << ':' << std::setfill('0')
                << std::setw(2) << tm->tm_min << ':' << std::setfill('0')
                << std::setw(2) << tm->tm_sec;
}

void grpc_log_func(gpr_log_func_args* args) {
  std::stringstream ss;
  ss << args->message << "===[" << args->file << ":" << args->line << "] ";
  GLOGV(ss.str());
  // log(ss.str(), args->severity != GPR_LOG_SEVERITY_ERROR);
  // std::cout << "[GRPC]" << ss.str() << std::endl;
}

bool init(const std::string logfile) {
  SLOGI("===> {}", logfile);
  // gpr_set_log_function(nullptr);
  gpr_set_log_function(&grpc_log_func);
  if (logfile.empty()) {
    SLOGI("FILE PATH EMPTY {}", logfile);
    return false;
  }
  g_logfile.open(logfile.c_str(), std::ios::out);
  if (!g_logfile.is_open()) {
    SLOGI("cannot open log {}", logfile);
    return false;
  }
  return true;
}

void unInit() { g_logfile.close(); }

void log(const std::string& message, bool verbose) {
  // if (verbose && !g_Verbose) {
  //  return;
  //}
  if (verbose) {
    if (g_logfile.is_open()) {
#if defined(OS_WIN)
      edu_timespec now = now_impl(EDU_CLOCK_REALTIME);
#if 0
		  struct tm tm;
		  char time_buffer[64];
		  time_t timer;
		  timer = (time_t)now.tv_sec;
		  if (localtime_s(&tm, &timer)) {
			strcpy(time_buffer, "error:localtime");
		  }
		  else if (0 == strftime(time_buffer, sizeof(time_buffer),
			"%m%d %H:%M:%S", &tm)) {
			strcpy(time_buffer, "error:strftime");
		  }
#endif
      g_logfile << "[" << getLocalTime()
                << "." /*<< std::setfill('0')*/
                       /*<< std::setw(3) */
                << (int)now.tv_nsec << "]" << GetCurrentThreadId() << message
                << "\n";
      /* << std::endl;*/
#else
      g_logfile << "[" << getLocalTime() << "]" << message << std::endl;
#endif
    } else {
      std::cout << "no logfile !!!" << std::endl;
    }
  } else {  //错误信息直接输出
    std::cout << message << std::endl;
  }
}
void set_verbose(bool verbose) { g_Verbose = verbose; }

std::ostream& get() { return g_logfile; }

}  // namespace EgcGrpcLog
}  // namespace EgcCommon