#ifndef EDU_SERVICE_MESH
#define EDU_SERVICE_MESH

#include <common/singleton.h>

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
    // grpc日志存放目录. 如果设置为"",不输出日志文件(
    // 统一使用unix风格的路径(/home/xxx/log/))
    std::string grpc_log_dir = "./grpc_log";
    // grpc日志等级
    std::string grpc_log_level = "debug";

    // sdk日志输出到控制台开关
    bool sdk_log_on_console = true;
    // sdk日志存放目录. 如果设置为"",不输出日志文件
    // 统一使用unix风格的路径(/home/xxx/log/))
    std::string sdk_log_dir = "./sdk_log";
    // sdl日志等级
    std::string sdk_log_level = "debug";
};

}  // namespace edu

#endif