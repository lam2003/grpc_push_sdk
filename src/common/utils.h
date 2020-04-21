#ifndef EDU_SERVICE_MESH_UTILS_H
#define EDU_SERVICE_MESH_UTILS_H

#include <string>

namespace edu {

class Utils {
  public:
    static std::string GetSystemTime(const std::string& format = "%Y-%m-%d");

};

}  // namespace edu

#endif