#ifndef EDU_PUSH_SDK
#define EDU_PUSH_SDK

#include <common/singleton.h>

#include <string>
#include <vector>

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
    std::string grpc_log_level = "trace";

    // sdk日志输出到控制台开关
    bool sdk_log_on_console = true;
    // sdl日志等级
    std::string sdk_log_level = "trace";

    // front_envoy 域名
    std::string      front_envoy_host  = "front.100.com";
    std::vector<int> front_envoy_ports = {14000, 15000};

    // hash header
    std::string route_hash_key   = "suid";
    std::string route_hash_value = "";
};

}  // namespace edu

#endif