#include <common/utils.h>

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
        return LOG_LEVEL::ERROR;
    }
    else if (strcasecmp(s.c_str(), "critical") == 0) {
        return LOG_LEVEL::CRITICAL;
    }
    else {
        // unknow level, default debug
        return LOG_LEVEL::DEBUG;
    }
}

}  // namespace edu