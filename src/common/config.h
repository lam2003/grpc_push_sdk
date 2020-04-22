#ifndef EDU_SERVICE_MESH
#define EDU_SERVICE_MESH

#include <common/singleton.h>

namespace edu {
class Config : public Singleton<Config> {
    friend Singleton<Config>;

  public:
    virtual ~Config() {}

  protected:
    Config()
    {
        // grpc日志输出到控制台开关
        grpc_log_on_console = true;
        // grpc日志等级
        grpc_log_level = "debug";

        // sdk日志输出到控制台开关
        sdk_log_on_console = true;
        // sdl日志等级
        sdk_log_level = "debug";
    }

  public:
    // grpc日志输出到控制台开关
    bool grpc_log_on_console;
    // grpc日志等级
    std::string grpc_log_level;

    // sdk日志输出到控制台开关
    bool sdk_log_on_console;
    // sdl日志等级
    std::string sdk_log_level;
};

}  // namespace edu

#endif