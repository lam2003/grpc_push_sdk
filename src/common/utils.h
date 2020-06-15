#ifndef EDU_PUSH_SDK_UTILS_H
#define EDU_PUSH_SDK_UTILS_H

#include <common/log.h>

#include <string>

namespace edu {

enum class TerminalType {
    UNKNOWN       = 0,
    WINDOWS_32    = 1,
    WINDOWS_64    = 2,
    IOS           = 3,
    ANDROID       = 4,
    IOS_SIMULATOR = 5,
    LINUX         = 6,
    UNIX          = 7,
    MAC           = 8,
    APPLE_UNKNOWN = 9,
    POSIX_UNKNOWN = 10
};

class Utils {
  public:
    static std::string  GetSystemTime(const std::string& format = "%Y-%m-%d");
    static int64_t      GetSteadyMilliSeconds();
    static int64_t      GetSteadyNanoSeconds();
    static TerminalType GetTerminalType();
    static uint64_t     GetSUID(uint32_t uid, uint64_t terminal_type);
    static int64_t      NanoSecondsToMilliSeconds(int64_t t);
    static std::string  CutFilePath(const std::string& filepath);
};

}  // namespace edu

#endif