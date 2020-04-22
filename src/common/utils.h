#ifndef EDU_SERVICE_MESH_UTILS_H
#define EDU_SERVICE_MESH_UTILS_H

#include <string>

#include <common/log.h>

namespace edu {

class Utils {
  public:
    static std::string GetSystemTime(const std::string& format = "%Y-%m-%d");
    static LOG_LEVEL   StrToLogLevel(const std::string& s);
};

}  // namespace edu

#endif