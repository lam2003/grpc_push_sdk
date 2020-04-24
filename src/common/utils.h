#ifndef EDU_PUSH_SDK_UTILS_H
#define EDU_PUSH_SDK_UTILS_H

#include <common/log.h>

#include <string>

namespace edu {

class Utils {
  public:
    static std::string GetSystemTime(const std::string& format = "%Y-%m-%d");
    static LOG_LEVEL   StrToLogLevel(const std::string& s);
    static int64_t     GetSteayMilliSeconds();
};

}  // namespace edu

#endif