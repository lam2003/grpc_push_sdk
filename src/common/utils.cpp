#include <chrono>
#include <common/utils.h>

namespace edu {

std::string Utils::GetSystemTime(const std::string& format)
{
    time_t t       = time(0);
    char   tmp[32] = {0};

    strftime(tmp, sizeof(tmp), format.c_str(), localtime(&t));
    return tmp;
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

std::string Utils::CutFilePath(const std::string& filepath)
{
    size_t pos = filepath.rfind(OS_SEGMENT);
    if (pos != std::string::npos) {
        return filepath.substr(pos + 1);
    }

    return filepath;
}

inline static uint8_t to_hex(uint8_t x)
{
    return x > 9 ? x + 55 : x + 48;
}

inline static uint8_t from_hex(uint8_t x)
{
    uint8_t y;
    if (x >= 'A' && x <= 'Z')
        y = x - 'A' + 10;
    else if (x >= 'a' && x <= 'z')
        y = x - 'a' + 10;
    else if (x >= '0' && x <= '9')
        y = x - '0';
    else
        assert(0);
    return y;
}

std::string Utils::URLEncode(const std::string& str)
{
    std::string str_temp = "";
    size_t      length   = str.length();
    for (size_t i = 0; i < length; i++) {
        if (isalnum((uint8_t)str[i]) || (str[i] == '-') || (str[i] == '_') ||
            (str[i] == '.') || (str[i] == '~'))
            str_temp += str[i];
        else if (str[i] == ' ')
            str_temp += "+";
        else {
            str_temp += '%';
            str_temp += to_hex((uint8_t)str[i] >> 4);
            str_temp += to_hex((uint8_t)str[i] % 16);
        }
    }
    return str_temp;
}

std::string Utils::GetPlatformName()
{
#ifdef _WIN32
    return "pc";
#elif __APPLE__
    return "ios";
#elif __ANDROID__
    return "android"
#elif __linux__
    return "linux";
#else
    return "unknow";
#endif
}

}  // namespace edu