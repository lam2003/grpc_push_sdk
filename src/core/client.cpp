#include <common/config.h>
#include <common/log.h>
#include <common/utils.h>
#include <core/client.h>
#include <push_sdk.h>

#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

#include <sstream>

#define CQ_WAIT_TIME 50  // ms
#define HASH_HEADER_KEY "suid"

namespace edu {

std::string channel_state_to_string(ChannelState state)
{
    switch (state) {
        case ChannelState::CONNECTED: return "CONNECTED";
        case ChannelState::DISCONNECTED: return "DISCONNECTED";
        case ChannelState::UNKNOW:
        default: return "UNKNOW";
    }
}

static std::string client_event_to_string(ClientEvent event)
{
    switch (event) {
        case ClientEvent::CONNECTED: return "CONNECTED";
        case ClientEvent::FINISHED: return "FINISHED";
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
    status_               = ClientStatus::WAIT_CONNECT;
    channel_state_        = ChannelState::UNKNOW;
    state_listener_       = nullptr;
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

void Client::SetChannelStateListener(
    std::shared_ptr<ChannelStateListener> listener)
{
    state_listener_ = listener;
}

static grpc::ChannelArguments get_channel_args()
{
    grpc::ChannelArguments args;
    // GRPC心跳间隔
    args.SetInt(GRPC_ARG_KEEPALIVE_TIME_MS, 1000);
    // GRPC心跳超时时间
    args.SetInt(GRPC_ARG_KEEPALIVE_TIMEOUT_MS, 5000);
    // 没有GRPC调用时也强制发送心跳
    args.SetInt(GRPC_ARG_KEEPALIVE_PERMIT_WITHOUT_CALLS, 1);
    // 在发送数据帧前，可以发送多少个心跳？不限制
    args.SetInt(GRPC_ARG_HTTP2_MAX_PINGS_WITHOUT_DATA, 0);
    // 发送连续的ping帧而不接收任何数据之间的最短时间
    args.SetInt(GRPC_ARG_HTTP2_MIN_SENT_PING_INTERVAL_WITHOUT_DATA_MS, 1000);

    return args;
}

void Client::create_channel()
{
    std::ostringstream oss;
    oss << Config::Instance()->front_envoy_host << ":"
        << Config::Instance()->front_envoy_ports
               [front_envoy_port_idx_ %
                Config::Instance()->front_envoy_ports.size()];
    front_envoy_port_idx_++;

    log_i("create channel with {}", oss.str());
    channel_ = grpc::CreateCustomChannel(
        oss.str(), grpc::InsecureChannelCredentials(), get_channel_args());
    stub_ = PushGateway::NewStub(channel_);
}

void Client::destroy_channel()
{
    if (stub_) {
        stub_.release();
        stub_ = nullptr;
    }

    if (channel_) {
        channel_.reset();
        channel_ = nullptr;
    }

    if (channel_state_ != ChannelState::DISCONNECTED && state_listener_) {
        state_listener_->OnChannelStateChange(ChannelState::DISCONNECTED);
        channel_state_ = ChannelState::UNKNOW;
    }
}

void Client::create_stream()
{
    ctx_.reset(new grpc::ClientContext());
    // 填入路由所需header kv
    ctx_->AddMetadata(HASH_HEADER_KEY, "");

    stream_ = stub_->AsyncPushRegister(
        ctx_.get(), cq_.get(), reinterpret_cast<void*>(ClientEvent::CONNECTED));
    status_ = ClientStatus::WAIT_CONNECT;

    last_heartbeat_ts_ = Utils::GetSteayMilliSeconds();
}

void Client::destroy_stream()
{
    if (ctx_) {
        ctx_->TryCancel();
        ctx_.release();
        ctx_ = nullptr;
    }
    if (stream_) {
        stream_.release();
        stream_ = nullptr;
    }
}

static PushData data;

void Client::handle_event(ClientEvent event)
{
    switch (event) {
        case ClientEvent::CONNECTED: {
            stream_->Read(&data,
                          reinterpret_cast<void*>(ClientEvent::READ_DONE));

            if (!queue_.empty()) {
                std::shared_ptr<PushRegReq> req = queue_.front();
                queue_.pop();

                stream_->Write(
                    *req, reinterpret_cast<void*>(ClientEvent::WRITE_DONE));

                status_ = ClientStatus::WAIT_WRITE_DONE;
            }
            else {
                status_ = ClientStatus::READY_TO_WRITE;
            }

            break;
        }
        case ClientEvent::WRITE_DONE: {
            if (!queue_.empty()) {
                std::shared_ptr<PushRegReq> req = queue_.front();
                queue_.pop();

                stream_->Write(
                    *req, reinterpret_cast<void*>(ClientEvent::WRITE_DONE));

                status_ = ClientStatus::WAIT_WRITE_DONE;
            }
            else {
                status_ = ClientStatus::READY_TO_WRITE;
            }

            break;
        }
        case ClientEvent::READ_DONE: {
            stream_->Read(&data,
                          reinterpret_cast<void*>(ClientEvent::READ_DONE));
            break;
        }
    }
}

void Client::check_channel_and_stream(bool ok)
{
    grpc_connectivity_state state = channel_->GetState(true);

    ChannelState now_channel_state;
    if (state == GRPC_CHANNEL_READY) {
        now_channel_state = ChannelState::CONNECTED;
    }
    else {
        now_channel_state = ChannelState::DISCONNECTED;
    }

    if (now_channel_state != channel_state_ && state_listener_) {
        state_listener_->OnChannelStateChange(now_channel_state);
        channel_state_ = now_channel_state;
    }

    if (state == GRPC_CHANNEL_TRANSIENT_FAILURE ||
        state == GRPC_CHANNEL_SHUTDOWN) {
        // 使用其他端口重连,避免某些端口被封
        log_e("channel error. going to rebuild channel and stream");
        destroy_stream();
        destroy_channel();
        create_channel();
        create_stream();
    }

    if (state == GRPC_CHANNEL_READY && !ok) {
        log_e("stream error. going to rebuild stream");
        destroy_stream();
        create_stream();
    }
}

void Client::handle_cq_timeout()
{
    int64_t now = Utils::GetSteayMilliSeconds();

    if (now - last_heartbeat_ts_ >= Config::Instance()->heart_beat_interval) {
        std::shared_ptr<PushRegReq> req = std::make_shared<PushRegReq>();
        req->set_uri(StreamURI::PPushGateWayPingURI);
        queue_.push(req);
        last_heartbeat_ts_ = now;
    }

    if (status_ == ClientStatus::READY_TO_WRITE && !queue_.empty()) {
        std::shared_ptr<PushRegReq> req = queue_.front();
        queue_.pop();
        stream_->Write(*req, reinterpret_cast<void*>(ClientEvent::WRITE_DONE));
    }
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
                log_w("completion queue shutdown");
                run_ = false;
                break;
            }
            case grpc::CompletionQueue::TIMEOUT: {
                check_channel_and_stream(ok);
                if (ok) {
                    handle_cq_timeout();
                }
                break;
            }
            case grpc::CompletionQueue::GOT_EVENT: {
                check_channel_and_stream(ok);
                if (ok) {
                    handle_event(event);
                }
                break;
            }
            default:
                log_e("completion queue return unknow status={}", status);
                break;
        }
    }

    // 收尾工作....
    log_i("event loop thread going to quit");
    destroy_stream();
    destroy_channel();
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
    }

    if (thread_) {
        thread_->join();
        thread_.release();
        thread_ = nullptr;
    }

    if (cq_) {
        cq_.release();
        cq_ = nullptr;
    }

    if (state_listener_) {
        state_listener_.reset();
        state_listener_ = nullptr;
    }
}
}  // namespace edu