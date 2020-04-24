#ifndef PUSH_SDK_CLIENT_H
#define PUSH_SDK_CLIENT_H

#include <proto/pushGateWay.grpc.pb.h>

#include <atomic>
#include <memory>
#include <queue>
#include <string>
#include <thread>
#include <vector>

using PushGateway = grpc::push::gateway::PushGateway;
using Stub        = grpc::push::gateway::PushGateway::Stub;
using PushRegReq  = grpc::push::gateway::PushRegReq;
using PushData    = grpc::push::gateway::PushData;
using StreamURI   = grpc::push::gateway::StreamURI;
using Stream =
    grpc::ClientAsyncReaderWriterInterface<grpc::push::gateway::PushRegReq,
                                           grpc::push::gateway::PushData>;

namespace edu {

enum class ClientEvent {
    CONNECTED  = 0x01,
    READ_DONE  = 0x02,
    WRITE_DONE = 0x03,
    FINISHED   = 0x04
};

enum class ClientStatus {
    WAIT_CONNECT,
    READY_TO_WRITE,
    WAIT_WRITE_DONE,
    WAIT_READ_DONE,
    FINISHED
};

enum class ChannelState { CONNECTED, DISCONNECTED, UNKNOW };

extern std::string channel_state_to_string(ChannelState state);

class ChannelStateListener {
  public:
    ChannelStateListener() {}
    virtual ~ChannelStateListener() {}

  public:
    virtual void OnChannelStateChange(ChannelState state) = 0;
};

class Client {
  public:
    Client();
    virtual ~Client();

    virtual int  Initialize();
    virtual void Destroy();

    virtual void
    SetChannelStateListener(std::shared_ptr<ChannelStateListener> listener);

  private:
    void create_channel();
    void destroy_channel();
    void create_stream();
    void destroy_stream();

    void event_loop();
    void handle_event(ClientEvent event);
    void handle_cq_timeout();
    void check_channel_and_stream(bool ok);

  private:
    int                                     front_envoy_port_idx_;
    int64_t                                 last_heartbeat_ts_;
    ClientStatus                            status_;
    ChannelState                            channel_state_;
    std::shared_ptr<ChannelStateListener>   state_listener_;
    std::queue<std::shared_ptr<PushRegReq>> queue_;
    std::unique_ptr<grpc::CompletionQueue>  cq_;
    std::shared_ptr<grpc::Channel>          channel_;
    std::unique_ptr<Stub>                   stub_;
    std::unique_ptr<Stream>                 stream_;
    std::unique_ptr<grpc::ClientContext>    ctx_;
    std::unique_ptr<std::thread>            thread_;
    std::atomic<bool>                       run_;
};
}  // namespace edu
#endif