#include <common/config.h>
#include <common/log.h>
#include <core/client.h>
#include <push_sdk.h>

#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

#include <sstream>

#define CQ_WAIT_TIME 50  // ms

namespace edu {

static std::string client_event_to_string(ClientEvent event)
{
    switch (event) {
        case ClientEvent::CONNECTED: return "CONNECTED";
        case ClientEvent::FINISH: return "FINISH";
        case ClientEvent::READ_DONE: return "READ_DONE";
        case ClientEvent::WRITE_DONE: return "WRITE_DONE";
        default: return "UNKNOW";
    }
}

static std::string grpc_channel_state_to_string(grpc_connectivity_state state)
{
    switch (state) {
        case GRPC_CHANNEL_IDLE: return "IDLE";
        case GRPC_CHANNEL_CONNECTING: return "CONNECTING";
        case GRPC_CHANNEL_READY: return "READY";
        case GRPC_CHANNEL_TRANSIENT_FAILURE: return "TRANSIENT_FAILURE";
        case GRPC_CHANNEL_SHUTDOWN: return "SHUTDOWN";
        default: return "UNKNOW";
    }
}

Client::Client()
{
    front_envoy_port_idx_ = 0;
    last_heartbeat_ts_    = -1;
    cq_ = std::unique_ptr<grpc::CompletionQueue>(new grpc::CompletionQueue);
    channel_ = nullptr;
    stub_    = nullptr;
    stream_  = nullptr;
    ctx_     = nullptr;
    thread_  = nullptr;
    run_     = false;
}

Client ::~Client()
{
    Destroy();
}

void Client::create_channel()
{
    std::ostringstream oss;
    oss << Config::Instance()->front_envoy_host << ":"
        << Config::Instance()->front_envoy_ports
               [front_envoy_port_idx_ %
                Config::Instance()->front_envoy_ports.size()];
    front_envoy_port_idx_++;

    log_d("create channel with {}", oss.str());
    channel_ =
        grpc::CreateChannel(oss.str(), grpc::InsecureChannelCredentials());
    stub_ = PushGateway::NewStub(channel_);
}

void Client::destroy_channel()
{
    if (stub_) {
        stub_.reset(nullptr);
    }

    if (channel_) {
        channel_.reset();
        channel_ = nullptr;
    }
}

void Client::create_stream()
{
    ctx_.reset(new grpc::ClientContext());
    // 填入路由所需header kv
    ctx_->AddMetadata(Config::Instance()->route_hash_key,
                      Config::Instance()->route_hash_value);

    stream_ = stub_->AsyncPushRegister(
        ctx_.get(), cq_.get(), reinterpret_cast<void*>(ClientEvent::CONNECTED));
}

void Client::destroy_stream()
{
    if (ctx_) {
        ctx_->TryCancel();
        ctx_.reset(nullptr);
    }
    if (stream_) {
        stream_.reset(nullptr);
    }
}

void Client::handle_event(bool ok, ClientEvent event) {}

void Client::handle_cq_error()
{
    grpc_connectivity_state state = channel_->GetState(true);
    log_e("completion queue error. grpc channel state={}",
          grpc_channel_state_to_string(state));

    if (state == GRPC_CHANNEL_TRANSIENT_FAILURE ||
        state == GRPC_CHANNEL_CONNECTING || GRPC_CHANNEL_SHUTDOWN) {
        // 使用其他端口重连,避免某些端口被封
        log_d("going to rebuild channel and stream");
        destroy_stream();
        destroy_channel();
        create_channel();
        create_stream();
    }
    else if (state == GRPC_CHANNEL_IDLE) {
        // 这个状态只会在连接还未建立就析构客户端时出现
        log_d("context cancelled before connection build");
        return;
    }
    else {
        // Stream发生错误，但是连接正常
        log_d("going to rebuild stream");
    }
}

int Client::handle_cq_timeout()
{
    int ret = PS_RET_SUCCESS;

    return ret;
}

void Client::event_loop()
{
    ClientEvent event;
    bool        ok;

    create_channel();
    create_stream();

    gpr_timespec tw = gpr_time_from_millis(CQ_WAIT_TIME, GPR_TIMESPAN);
    grpc::CompletionQueue::NextStatus status;
    while (run_) {
        status = cq_->AsyncNext(reinterpret_cast<void**>(&event), &ok, tw);
        switch (status) {
            case grpc::CompletionQueue::SHUTDOWN: {
                log_w("grpc completion queue shutdown");
                run_ = false;
                break;
            }
            case grpc::CompletionQueue::TIMEOUT: {
                handle_cq_timeout();
                break;
            }
            case grpc::CompletionQueue::GOT_EVENT: {
                if (!ok) {
                    handle_cq_error();
                }
                else {
                    log_d("grpc completion queue got event {}",
                          client_event_to_string(event));
                    handle_event(ok, event);
                }

                break;
            }
            default:
                log_e("grpc completion queue return unknow status={}", status);
                break;
        }
    }
    log_i("event loop thread going to quit");
    // 收尾工作....
}

int Client::Initialize()
{
    int ret = PS_RET_SUCCESS;

    run_    = true;
    thread_ = std::unique_ptr<std::thread>(
        new std::thread(std::bind(&Client::event_loop, this)));

    return ret;
}

void Client::Destroy()
{
    run_ = false;

    if (cq_) {
        cq_->Shutdown();
        cq_.reset();
        cq_ = nullptr;
    }

    if (thread_) {
        thread_->join();
        thread_.reset();
        thread_ = nullptr;
    }

    destroy_stream();
    destroy_channel();
    
}
}  // namespace edu