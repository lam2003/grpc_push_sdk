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
    // state_listener_ = listener;
}

void Client::SetClientStatusListener(
    std::shared_ptr<ClientStatusListener> listener)
{
    // status_listener_ = listener;
}

void Client::SetMessageHandler(std::shared_ptr<MessageHandler> hdl)
{
    // msg_hdl_ = hdl;
}

void Client::CleanQueue()
{
    // std::unique_lock<std::mutex> lock(mux_);
    // while (!queue_.empty()) {
    //     queue_.pop();
    // }
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
    if (channel_state_lis_) {
        channel_state_lis_->NotifyChannelState(state == GRPC_CHANNEL_READY ?
                                                   ChannelState::OK :
                                                   ChannelState::NO_READY);
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

int Client::Initialize(uint32_t uid, uint64_t suid)
{
    int ret = PS_RET_SUCCESS;

    if (init_) {
        log_w("client already initialized");
        return PS_RET_ALREADY_INIT;
    }

    uid_  = uid;
    suid_ = suid;

    // 创建Channel
    create_channel_and_stub();

    thread_ = std::unique_ptr<std::thread>(new std::thread([this]() {
        ClientEvent event;
        bool        ok;

        while (cq->Next(reinterpret_cast<void**>(&event), &ok)) {
            check_and_notify_channel_state();

            if (event == ClientEvent::FINISHED) {
                check_and_reconnect();
                create_and_init_stream();
                continue;
            }

            if (!ok) {
                assert(st_);
                st_->Finish();
                continue;
            }

            assert(st_);
            st_->Process(event);
        }
    }));

    return ret;
}

void Client::Send(std::shared_ptr<PushRegReq> req)
{
    if (st_) {
        st_->Send(req);
    }
}

void Client::Destroy() {}
}  // namespace edu