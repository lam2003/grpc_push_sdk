#ifndef EDU_SERVICE_MESH
#define EDU_SERVICE_MESH

#include <common/singleton.h>

#include <string>

namespace edu {
class Config : public Singleton<Config> {
    friend Singleton<Config>;

  public:
    virtual ~Config() {}

  protected:
    Config() {}

  public:
    // grpc日志输出到控制台开关
    bool grpc_log_on_console = true;
    // grpc日志等级
    std::string grpc_log_level = "debug";

    // sdk日志输出到控制台开关
    bool sdk_log_on_console = true;
    // sdl日志等级
    std::string sdk_log_level = "debug";
};

}  // namespace edu

#endif