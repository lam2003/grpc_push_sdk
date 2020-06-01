#include <common/config.h>
#include <common/log.h>
#include <common/utils.h>
#include <core/client.h>
#include <push_sdk.h>

#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

#include <sstream>

#include <core/stream.h>

#define HASH_HEADER_KEY "suid"
#define UID_HEADER_KEY "uid"

namespace edu {

std::atomic<uint32_t> Client::port_index_(0);

Client::Client()
{
    cq      = std::unique_ptr<grpc::CompletionQueue>(new grpc::CompletionQueue);
    stub    = nullptr;
    channel = nullptr;

    st_   = nullptr;
    init_ = false;

    uid_  = 0;
    suid_ = 0;
}

Client ::~Client() {}

void Client::SetChannelStateListener(
    std::shared_ptr<ChannelStateListener> listener)
{
    channel_state_lis_ = listener;
}

void Client::SetClientStatusListener(
    std::shared_ptr<ClientStatusListener> listener)
{
    status_lis_ = listener;
}

void Client::SetMessageHandler(std::shared_ptr<MessageHandler> hdl)
{
    msg_hdl_ = hdl;
}

void Client::CleanQueue()
{
    mux_.lock();
    msg_queue_.clear();
    mux_.unlock();
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

void Client::create_channel_and_stub()
{
    std::ostringstream oss;
    oss << Config::Instance()->front_envoy_host << ":"
        << Config::Instance()->front_envoy_ports
               [port_index_++ % Config::Instance()->front_envoy_ports.size()];

    log_t("trying connect to {}", oss.str());

    channel = grpc::CreateCustomChannel(
        oss.str(), grpc::InsecureChannelCredentials(), get_channel_args());

    assert(channel);

    stub = PushGateway::NewStub(channel);

    assert(stub);
}

void Client::create_and_init_stream()
{
    assert(stub);
    st_ = std::unique_ptr<Stream>(new Stream(shared_from_this()));
    assert(st_);
    st_->Init();
}

void Client::check_and_notify_channel_state()
{
    assert(channel);
    grpc_connectivity_state state = channel->GetState(false);

    ChannelState new_state =
        state == GRPC_CHANNEL_READY ? ChannelState::OK : ChannelState::NO_READY;

    if (new_state != last_channel_state_) {
        last_channel_state_ = new_state;
    }
    else {
        return;
    }

    if (channel_state_lis_) {
        channel_state_lis_->NotifyChannelState(last_channel_state_);
    }
}
void Client::check_and_reconnect()
{
    assert(channel);
    grpc_connectivity_state state = channel->GetState(false);
    if (state == GRPC_CHANNEL_SHUTDOWN ||
        state == GRPC_CHANNEL_TRANSIENT_FAILURE) {
        create_channel_and_stub();
    }
}

void Client::on_connected()
{
    if (status_lis_) {
        status_lis_->OnConnected();
    }
}

void Client::on_read(std::shared_ptr<PushData> push_data)
{
    if (msg_hdl_) {
        msg_hdl_->OnMessage(push_data);
    }
}

void Client::send_all_msgs()
{
    int64_t now = Utils::GetSteadyMilliSeconds();

    if (now - last_heartbeat_ts_ >= Config::Instance()->heart_beat_interval) {
        std::shared_ptr<PushRegReq> req = std::make_shared<PushRegReq>();
        req->set_uri(StreamURI::PPushGateWayPingURI);
        st_->Send(req);
        last_heartbeat_ts_ = now;
    }

    mux_.lock();
    st_->SendMsgs(msg_queue_);
    mux_.unlock();
}

int Client::Initialize(uint32_t uid, uint64_t suid)
{
    int ret = PS_RET_SUCCESS;

    if (init_) {
        log_w("client already initialized");
        return PS_RET_ALREADY_INIT;
    }

    last_heartbeat_ts_ = 0;
    uid_               = uid;
    suid_              = suid;

    run_ = true;

    thread_ = std::unique_ptr<std::thread>(new std::thread([this]() {
        gpr_timespec tw = gpr_time_from_millis(
            Config::Instance()->grpc_cq_timeout_ms, GPR_TIMESPAN);
        grpc::CompletionQueue::NextStatus status;
        ClientEvent                       event;
        bool                              ok;

        create_channel_and_stub();
        create_and_init_stream();

        while (run_) {
            status = cq->AsyncNext(reinterpret_cast<void**>(&event), &ok, tw);

            check_and_notify_channel_state();

            switch (status) {
                case grpc::CompletionQueue::SHUTDOWN: {
                    log_w("completion queue shutdown");
                    run_ = false;
                    break;
                }
                case grpc::CompletionQueue::TIMEOUT: {
                    if (!ok && st_->IsConnected()) {
                        st_->Finish();
                        continue;
                    }

                    if (st_->IsReadyToSend()) {
                        send_all_msgs();
                    }

                    break;
                }
                case grpc::CompletionQueue::GOT_EVENT: {
                    log_t("stream event={}", event);
                    if (event == ClientEvent::FINISHED) {
                        if (status_lis_) {
                            status_lis_->OnFinish(st_->LastRequest(),
                                                  st_->GrpcStatus());
                        }
                        check_and_reconnect();
                        create_and_init_stream();
                        continue;
                    }

                    if (!ok && st_->IsConnected()) {
                        st_->Finish();
                        continue;
                    }

                    if (st_->IsReadyToSend()) {
                        send_all_msgs();
                    }

                    st_->Process(event);

                    break;
                }
                default: {
                    break;
                }
            }
        }
    }));

    return ret;
}

void Client::Send(std::shared_ptr<PushRegReq> req)
{
    mux_.lock();
    msg_queue_.emplace_back(req);
    mux_.unlock();
}

void Client::Destroy() {}
}  // namespace edu