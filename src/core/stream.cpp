#include <common/config.h>
#include <common/utils.h>
#include <core/client.h>
#include <core/stream.h>

#include <sstream>

#include <grpc++/grpc++.h>

namespace edu {

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

std::atomic<uint32_t> Stream::port_index_(0);

Stream::Stream(std::shared_ptr<Client> client)
{
    client_ = client;
    ctx_    = std::unique_ptr<grpc::ClientContext>(new grpc::ClientContext);

    std::ostringstream oss;
    oss << Config::Instance()->front_envoy_host << ":"
        << Config::Instance()->front_envoy_ports
               [port_index_ % Config::Instance()->front_envoy_ports.size()];
    port_index_++;
    channel_ = grpc::CreateCustomChannel(
        oss.str(), grpc::InsecureChannelCredentials(), get_channel_args());
    stub_      = PushGateway::NewStub(channel_);
    push_data_ = std::unique_ptr<PushData>(new PushData);
    listener_  = nullptr;
    rw_        = nullptr;
}

Stream::~Stream() {}

void Stream::Init()
{
    status_             = StreamStatus::WAIT_CONNECT;
    need_to_finish_     = false;
    last_heart_beat_ts_ = Utils::GetSteadyMilliSeconds();

    rw_ = stub_->AsyncPushRegister(
        ctx_.get(), client_->cq.get(),
        reinterpret_cast<void*>(ClientEvent::CONNECTED));
}

void Stream::Process(ClientEvent event)
{
    std::unique_lock<std::mutex> lock(mux_);

    switch (event) {
        case ClientEvent::CONNECTED: {
            rw_->Read(push_data_.get(),
                      reinterpret_cast<void*>(ClientEvent::READ_DONE));
            status_ = StreamStatus::READY_TO_WRITE;
            lock.unlock();

            if (listener_) {
                listener_->OnConnected();
            }
            break;
        }
        case ClientEvent::READ_DONE: {
            rw_->Read(push_data_.get(),
                      reinterpret_cast<void*>(ClientEvent::READ_DONE));
            lock.unlock();

            if (listener_) {
                PushData*                 p = new PushData(*push_data_.get());
                std::shared_ptr<PushData> push_data(p);
                listener_->OnRead(push_data);
            }
            break;
        }
        case ClientEvent::WRITE_DONE: {
            if (msg_queue_.empty()) {
                status_ = StreamStatus::READY_TO_WRITE;
                if (need_to_finish_) {
                    lock.unlock();
                    Finish();
                }
            }
            else {
                rw_->Write(*msg_queue_.front(),
                           reinterpret_cast<void*>(ClientEvent::WRITE_DONE));
                status_ = StreamStatus::WAIT_WRITE_DONE;
                msg_queue_.pop_front();
            }
            break;
        }
        default: {
            break;
        }
    }
}

void Stream::Send(std::shared_ptr<PushRegReq> req)
{
    std::unique_lock<std::mutex> lock(mux_);

    if (status_ != StreamStatus::READY_TO_WRITE &&
        status_ != StreamStatus::WAIT_WRITE_DONE) {
        return;
    }

    if (status_ != StreamStatus::READY_TO_WRITE) {
        msg_queue_.emplace_back(req);
    }
    else {
        rw_->Write(*req, reinterpret_cast<void*>(ClientEvent::WRITE_DONE));
        status_ = StreamStatus::WAIT_WRITE_DONE;
    }
}

void Stream::Finish()
{
    // 在CQ返回不OK时，也尝试发送finish
    std::unique_lock<std::mutex> lock(mux_);

    if (status_ == StreamStatus::WAIT_CONNECT ||
        status_ == StreamStatus::FINISHED) {
        return;
    }

    if (status_ == StreamStatus::READY_TO_WRITE) {
        // 此时队列为空,直接finish,否则设置退出标志
        rw_->Finish(&grpc_status_,
                    reinterpret_cast<void*>(ClientEvent::FINISHED));

        status_ = StreamStatus::FINISHED;

        lock.unlock();
    }
    else {
        need_to_finish_ = true;
    }
}

}  // namespace edu