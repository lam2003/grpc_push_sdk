#ifndef EDU_PUSH_SDK_CONFIG_H
#define EDU_PUSH_SDK_CONFIG_H

#include <common/singleton.h>

#include <map>
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
#ifdef __linux__
    // logger flush到文件的时间间隔(sec)
    int logger_flush_interval_sec = 1;
    // grpc日志输出到控制台开关
    bool grpc_log_on_console = false;
    // grpc日志等级
    std::string grpc_log_level = "trace";

    // sdk日志输出到控制台开关
    bool sdk_log_on_console = true;
    // sdk日志等级
    std::string sdk_log_level = "trace";

    // front_envoy 域名
    std::string front_envoy_host = "183.36.117.51";
#else
    // logger flush到文件的时间间隔(sec)
    int logger_flush_interval_sec = 1;
    // grpc日志输出到控制台开关
    bool grpc_log_on_console = false;
    // grpc日志等级
    std::string grpc_log_level = "trace";

    // sdk日志输出到控制台开关
    bool sdk_log_on_console = false;
    // sdk日志等级
    std::string sdk_log_level = "trace";

    // front_envoy 域名
    std::string front_envoy_host = "front.100.com";
#endif
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
    int grpc_cq_timeout_ms = 50;
    // GRPC 等待连接成功的时间(ms)
    int grpc_wait_connect_ms = 500;

    // PushGateway call超时时长(ms)
    int call_timeout_interval = 3000;
    // PushGateway 检测超时间隔(ms)
    int call_check_timeout_interval = 500;

    // ELK project
    std::string elk_project_name = "100edu-signal-platform";
    // ELK region
    std::string elk_region = "cn-shenzhen";
    // ELK logStore
    std::string elk_log_store = "edu_sp_push_sdk_test";
    // ELK source 客户端无法拿到外网IP
    std::string elk_source = "localhost";
    // ELK encode 1代表base64，2代表URLEncode
    int elk_encode = 2;
    // ELK upload interval
    int elk_upload_interval_ms = 5000;
    // ELK upload min size
    int elk_upload_min_size = 5;
    // ELK upload max size
    int elk_upload_max_size = 100;
    // ELK upload host
    std::string elk_upload_host = "cloud-log.yy.com";
    // ELK upload path
    std::string elk_upload_path = "/api/log/put";
    // ELK upload headers
    std::map<std::string, std::string> elk_upload_headers = {
        {"referer", "www.yy.com"},
    };
};

}  // namespace edu

#endif