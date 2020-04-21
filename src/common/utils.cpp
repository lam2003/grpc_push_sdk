#include <common/utils.h>

namespace edu {

std::string Utils::GetSystemTime(const std::string& format)
{
    time_t t       = time(0);
    char   tmp[32] = {NULL};

    strftime(tmp, sizeof(tmp), format.c_str(), localtime(&t));
    return tmp;
}

}  // namespace edu