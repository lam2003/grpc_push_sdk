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
    bool grpc_log_on_console = false;
    // grpc日志等级
    std::string grpc_log_level = "trace";

    // sdk日志输出到控制台开关
    bool sdk_log_on_console = false;
    // sdl日志等级
    std::string sdk_log_level = "trace";

    // front_envoy 域名
    std::string      front_envoy_host  = "front.100.com";
    std::vector<int> front_envoy_ports = {15000, 14000, 5000, 1500, 500};

    // 与PushGateway心跳间隔(ms)
    int64_t heart_beat_interval = 3 * 1000;

    // GRPC心跳间隔(ms)
    int grpc_keep_alive_time = 1000;
    // GRPC心跳超时时间(ms)
    int grpc_keep_alive_timeout = 5000;
    // GRPC在没有调用时也强制发送心跳(1为enabled 0为disabled)
    int grpc_keep_alive_permit_without_calls = 1;
    // GRPC在发送数据帧前，可以发送多少个心跳？(0为不限制)
    int grpc_max_pings_without_data = 0;
    // GRPC发送连续的ping帧而不接收任何数据之间的最短时间(ms)
    int grpc_min_sent_ping_interval_without_data = 1000;
    // GRPC CQ等待事件超时时间(ms)
    int grpc_cq_timeout = 50;

    // PushGateway call超时时长(ms)
    int call_timeout_interval = 3000;
    // PushGateway 检测超时间隔(ms)
    int call_check_timeout_interval = 500;
};

}  // namespace edu

#endif