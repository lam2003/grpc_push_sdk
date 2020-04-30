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
        case ChannelState::OK: return "OK";
        case ChannelState::NO_READY: return "NO_READY";
        default: return "UNKNOW";
    }
}

std::string client_status_to_string(ClientStatus status)
{
    switch (status) {
        case ClientStatus::FINISHED: return "FINISHED";
        case ClientStatus::READY_TO_WRITE: return "READY_TO_WRITE";
        case ClientStatus::WAIT_CONNECT: return "WAIT_CONNECT";
        case ClientStatus::WAIT_WRITE_DONE: return "WAIT_WRITE_DONE";
        case ClientStatus::CONNECTED: return "CONNECTED";
        default: return "UNKNOW";
    }
}

std::string stream_uri_to_string(StreamURI uri)
{
    switch (uri) {
        case StreamURI::PPushGateWayLoginURI: return "PPushGateWayLoginURI";
        case StreamURI::PPushGateWayLoginResURI:
            return "PPushGateWayLoginResURI";
        case StreamURI::PPushGateWayLogoutURI: return "PPushGateWayLogoutURI";

        case StreamURI::PPushGateWayLogoutResURI:
            return "PPushGateWayLogoutResURI";
        case StreamURI::PPushGateWayJoinGroupURI:
            return "PPushGateWayJoinGroupURI";
        case StreamURI::PPushGateWayJoinGroupResURI:
            return "PPushGateWayJoinGroupResURI";
        case StreamURI::PPushGateWayLeaveGroupURI:
            return "PPushGateWayLeaveGroupURI";
        case StreamURI::PPushGateWayLeaveGroupResURI:
            return "PPushGateWayLeaveGroupResURI";
        case StreamURI::PPushGateWayPingURI: return "PPushGateWayPingURI";
        case StreamURI::PPushGateWayPongURI: return "PPushGateWayPongURI";
        case StreamURI::PPushGateWayNotifyToCloseURI:
            return "PPushGateWayNotifyToCloseURI";
        case StreamURI::PPushGateWayPushDataByUidURI:
            return "PPushGateWayPushDataByUidURI";
        case StreamURI::PPushGateWayPushDataByGroupURI:
            return "PPushGateWayPushDataByGroupURI";
        case StreamURI::PPushGateWayUNKNOWN:
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
    suid_                 = 0;
    front_envoy_port_idx_ = 0;
    last_heartbeat_ts_    = -1;
    client_status_        = ClientStatus::FINISHED;
    channel_state_        = ChannelState::NO_READY;
    state_listener_       = nullptr;
    status_listener_      = nullptr;
    msg_hdl_              = nullptr;
    msg_cache_            = nullptr;
    cq_                   = nullptr;
    channel_              = nullptr;
    stub_                 = nullptr;
    stream_               = nullptr;
    ctx_                  = nullptr;
    thread_               = nullptr;
    run_                  = false;
    init_                 = false;
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

void Client::SetClientStatusListener(
    std::shared_ptr<ClientStatusListener> listener)
{
    status_listener_ = listener;
}

void Client::SetMessageHandler(std::shared_ptr<MessageHandler> hdl)
{
    msg_hdl_ = hdl;
}

static grpc::ChannelArguments get_channel_args()
{
    grpc::ChannelArguments args;
    // GRPC心跳间隔(ms)
    args.SetInt(GRPC_ARG_KEEPALIVE_TIME_MS,
                Config::Instance()->grpc_keep_alive_time);
    // GRPC心跳超时时间(ms)
    args.SetInt(GRPC_ARG_KEEPALIVE_TIMEOUT_MS,
                Config::Instance()->grpc_keep_alive_timeout);
    // GRPC在没有调用时也强制发送心跳(1为enabled 0为disabled)
    args.SetInt(GRPC_ARG_KEEPALIVE_PERMIT_WITHOUT_CALLS,
                Config::Instance()->grpc_keep_alive_permit_without_calls);
    // GRPC在发送数据帧前，可以发送多少个心跳？(0为不限制)
    args.SetInt(GRPC_ARG_HTTP2_MAX_PINGS_WITHOUT_DATA,
                Config::Instance()->grpc_max_pings_without_data);
    // GRPC发送连续的ping帧而不接收任何数据之间的最短时间(ms)
    args.SetInt(GRPC_ARG_HTTP2_MIN_SENT_PING_INTERVAL_WITHOUT_DATA_MS,
                Config::Instance()->grpc_min_sent_ping_interval_without_data);

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

    log_i("connect to {}", oss.str());
    channel_ = grpc::CreateCustomChannel(
        oss.str(), grpc::InsecureChannelCredentials(), get_channel_args());
    stub_ = PushGateway::NewStub(channel_);
}

void Client::check_and_notify_channel_state_change(ChannelState new_state)
{
    if (channel_state_ != new_state && state_listener_) {
        state_listener_->OnChannelStateChange(new_state);
        channel_state_ = new_state;
    }
}

void Client::Send(std::shared_ptr<PushRegReq> req)
{
    std::unique_lock<std::mutex> lock(mux_);
    queue_.push(req);
}

void Client::CleanQueue()
{
    std::unique_lock<std::mutex> lock(mux_);
    while (!queue_.empty()) {
        queue_.pop();
    }
}

void Client::destroy_channel()
{
    stub_.release();
    stub_ = nullptr;

    channel_.reset();
    channel_ = nullptr;

    check_and_notify_channel_state_change(ChannelState::NO_READY);
}

void Client::check_and_notify_client_status_change(ClientStatus new_status)
{
    if (client_status_ != new_status && status_listener_) {
        status_listener_->OnClientStatusChange(new_status);
        client_status_ = new_status;
    }
}

void Client::create_stream()
{
    ctx_.reset(new grpc::ClientContext());
    // 填入路由所需header kv
    ctx_->AddMetadata(HASH_HEADER_KEY, std::to_string(suid_));

    stream_ = stub_->AsyncPushRegister(
        ctx_.get(), cq_.get(), reinterpret_cast<void*>(ClientEvent::CONNECTED));
    check_and_notify_client_status_change(ClientStatus::WAIT_CONNECT);

    last_heartbeat_ts_ = Utils::GetSteadyMilliSeconds();
}

void Client::destroy_stream()
{
    if (ctx_) {
        ctx_->TryCancel();
    }

    stream_.release();
    stream_ = nullptr;

    ctx_.release();
    ctx_ = nullptr;

    check_and_notify_client_status_change(ClientStatus::FINISHED);
}

void Client::handle_event(ClientEvent event)
{
    switch (event) {
        case ClientEvent::CONNECTED: {
            check_and_notify_client_status_change(ClientStatus::CONNECTED);

            msg_cache_ = std::make_shared<PushData>();
            stream_->Read(msg_cache_.get(),
                          reinterpret_cast<void*>(ClientEvent::READ_DONE));

            std::unique_lock<std::mutex> lock(mux_);
            if (!queue_.empty()) {
                std::shared_ptr<PushRegReq> req = queue_.front();
                queue_.pop();
                lock.unlock();

                stream_->Write(
                    *req, reinterpret_cast<void*>(ClientEvent::WRITE_DONE));

                check_and_notify_client_status_change(
                    ClientStatus::WAIT_WRITE_DONE);
            }
            else {
                check_and_notify_client_status_change(
                    ClientStatus::READY_TO_WRITE);
            }

            break;
        }
        case ClientEvent::WRITE_DONE: {
            std::unique_lock<std::mutex> lock(mux_);
            if (!queue_.empty()) {
                std::shared_ptr<PushRegReq> req = queue_.front();
                queue_.pop();
                lock.unlock();

                stream_->Write(
                    *req, reinterpret_cast<void*>(ClientEvent::WRITE_DONE));

                check_and_notify_client_status_change(
                    ClientStatus::WAIT_WRITE_DONE);
            }
            else {
                check_and_notify_client_status_change(
                    ClientStatus::READY_TO_WRITE);
            }

            break;
        }
        case ClientEvent::READ_DONE: {
            if (msg_hdl_ &&
                msg_cache_->uri() != StreamURI::PPushGateWayPongURI) {
                msg_hdl_->OnMessage(msg_cache_);
            }
            msg_cache_ = std::make_shared<PushData>();

            stream_->Read(msg_cache_.get(),
                          reinterpret_cast<void*>(ClientEvent::READ_DONE));
            break;
        }
        default: break;
    }
}

void Client::try_to_send_ping()
{
    int64_t now = Utils::GetSteadyMilliSeconds();

    if (now - last_heartbeat_ts_ >= Config::Instance()->heart_beat_interval) {
        std::shared_ptr<PushRegReq> req = std::make_shared<PushRegReq>();
        req->set_uri(StreamURI::PPushGateWayPingURI);
        mux_.lock();
        queue_.push(req);
        mux_.unlock();
        last_heartbeat_ts_ = now;
    }
}

void Client::check_channel_and_stream(bool ok)
{
    grpc_connectivity_state state = channel_->GetState(true);

    ChannelState new_channel_state;
    if (state == GRPC_CHANNEL_READY) {
        new_channel_state = ChannelState::OK;
    }
    else {
        new_channel_state = ChannelState::NO_READY;
    }

    check_and_notify_channel_state_change(new_channel_state);

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
    std::unique_lock<std::mutex> lock(mux_);
    if (client_status_ == ClientStatus::READY_TO_WRITE && !queue_.empty()) {
        check_and_notify_client_status_change(ClientStatus::WAIT_WRITE_DONE);
        std::shared_ptr<PushRegReq> req = queue_.front();
        queue_.pop();
        lock.unlock();
        stream_->Write(*req, reinterpret_cast<void*>(ClientEvent::WRITE_DONE));
    }
}

void Client::event_loop()
{
    ClientEvent event;
    bool        ok;

    cq_ = std::unique_ptr<grpc::CompletionQueue>(new grpc::CompletionQueue);

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
                    try_to_send_ping();
                    handle_cq_timeout();
                }
                break;
            }
            case grpc::CompletionQueue::GOT_EVENT: {
                check_channel_and_stream(ok);
                if (ok) {
                    try_to_send_ping();
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

    cq_->Shutdown();
    cq_.release();
    cq_ = nullptr;
}

int Client::Initialize(uint64_t suid)
{
    int ret = PS_RET_SUCCESS;

    if (init_) {
        ret = PS_RET_ALREADY_INIT;
        return ret;
    }

    suid_ = suid;

    run_    = true;
    thread_ = std::unique_ptr<std::thread>(
        new std::thread(std::bind(&Client::event_loop, this)));

    init_ = true;
    return ret;
}

void Client::Destroy()
{
    if (!init_) {
        return;
    }

    run_ = false;

    if (thread_) {
        thread_->join();
    }

    thread_.release();
    thread_ = nullptr;

    CleanQueue();

    state_listener_.reset();
    state_listener_ = nullptr;

    status_listener_.reset();
    status_listener_ = nullptr;

    msg_hdl_.reset();
    msg_hdl_ = nullptr;

    msg_cache_.reset();
    msg_cache_ = nullptr;

    init_ = false;
}
}  // namespace edu