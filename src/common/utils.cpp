#include <chrono>
#include <common/utils.h>

#ifdef _MSC_VER
#define strcasecmp stricmp
#define strncasecmp  strnicmp
#endif

namespace edu {

std::string Utils::GetSystemTime(const std::string& format)
{
    time_t t       = time(0);
    char   tmp[32] = {0};

    strftime(tmp, sizeof(tmp), format.c_str(), localtime(&t));
    return tmp;
}

LOG_LEVEL Utils::StrToLogLevel(const std::string& s)
{
    if (strcasecmp(s.c_str(), "trace") == 0) {
        return LOG_LEVEL::TRACE;
    }
    else if (strcasecmp(s.c_str(), "debug") == 0) {
        return LOG_LEVEL::DEBUG;
    }
    else if (strcasecmp(s.c_str(), "info") == 0) {
        return LOG_LEVEL::INFO;
    }
    else if (strcasecmp(s.c_str(), "warn") == 0) {
        return LOG_LEVEL::WARN;
    }
    else if (strcasecmp(s.c_str(), "error") == 0) {
        return LOG_LEVEL::ERROR2;
    }
    else if (strcasecmp(s.c_str(), "critical") == 0) {
        return LOG_LEVEL::CRITICAL;
    }
    else {
        // unknow level, default debug
        return LOG_LEVEL::DEBUG;
    }
}

int64_t Utils::GetSteadyMilliSeconds()
{
    using namespace std::chrono;
    steady_clock::time_point now = steady_clock::now();
    return duration_cast<milliseconds>(now.time_since_epoch()).count();
}

int64_t Utils::GetSteadyNanoSeconds()
{
    using namespace std::chrono;
    steady_clock::time_point now = steady_clock::now();
    return duration_cast<nanoseconds>(now.time_since_epoch()).count();
}

TerminalType Utils::GetTerminalType()
{
    TerminalType type = TerminalType::UNKNOWN;
#ifdef _WIN32
#    ifdef _WIN64
    type = TerminalType::WINDOWS_64;
#    else
    type = TerminalType::WINDOWS_32;
#    endif

#elif __APPLE__
#    include "TargetConditionals.h"

#    if TARGET_IPHONE_SIMULATOR
    type = TerminalType::IOS_SIMULATOR;
#    elif TARGET_OS_IPHONE
    type = TerminalType::IOS;
#    elif TARGET_OS_MAC
    type = TerminalType::MAC;
#    else
#        warning "Unknown Apple platform"
    type = TerminalType::APPLE_UNKNOWN;
#    endif
#elif __ANDROID__
    type = TerminalType::ANDROID;
#elif __linux__
    type = TerminalType::LINUX;
#elif __unix__
    type = TerminalType::UNIX;
#elif defined(_POSIX_VERSION)
    type = TerminalType::POSIX_UNKNOWN;
#else
#    warning "Unknown compiler"
    type = TerminalType::UNKNOWN;
#endif
    return type;
}

uint64_t Utils::GetSUID(uint32_t uid, uint64_t terminal_type)
{
    uint64_t suid = 0;
    uint64_t type = terminal_type;

    suid |= type << (32 + 8);
    suid |= uint64_t(uid) & 0x00000000FFFFFFFF;
    return suid;
}

int64_t Utils::NanoSecondsToMilliSeconds(int64_t t)
{
    return (t / 1000000);
}

}  // namespace edu