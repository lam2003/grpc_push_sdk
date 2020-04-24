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
    CONNECTED  = 1,
    READ_DONE  = 2,
    WRITE_DONE = 3,
    FINISHED   = 4
};

enum class ClientStatus {
    WAIT_CONNECT    = 100,
    READY_TO_WRITE  = 101,
    WAIT_WRITE_DONE = 102,
    FINISHED        = 103
};

enum class ChannelState { CONNECTED, DISCONNECTED };

extern std::string channel_state_to_string(ChannelState state);
extern std::string client_status_to_string(ClientStatus status);

class ChannelStateListener {
  public:
    ChannelStateListener() {}
    virtual ~ChannelStateListener() {}

  public:
    virtual void OnChannelStateChange(ChannelState state) = 0;
};

class ClientStatusListener {
  public:
    ClientStatusListener() {}
    ~ClientStatusListener() {}

  public:
    virtual void OnClientStatusChange(ClientStatus statue) = 0;
};

class Client {
  public:
    Client();
    virtual ~Client();

    virtual int  Initialize(uint32_t uid);
    virtual void Destroy();

    virtual void
    SetChannelStateListener(std::shared_ptr<ChannelStateListener> listener);

    virtual void
    SetClientStatusListener(std::shared_ptr<ClientStatusListener> listener);

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
    std::string                             uid_;
    int                                     front_envoy_port_idx_;
    int64_t                                 last_heartbeat_ts_;
    ClientStatus                            status_;
    ChannelState                            channel_state_;
    std::shared_ptr<ChannelStateListener>   state_listener_;
    std::shared_ptr<ClientStatusListener>   status_listener_;
    std::queue<std::shared_ptr<PushRegReq>> queue_;
    std::unique_ptr<grpc::CompletionQueue>  cq_;
    std::shared_ptr<grpc::Channel>          channel_;
    std::unique_ptr<Stub>                   stub_;
    std::unique_ptr<Stream>                 stream_;
    std::unique_ptr<grpc::ClientContext>    ctx_;
    std::unique_ptr<std::thread>            thread_;
    std::atomic<bool>                       run_;
    bool                                    init_;
};
}  // namespace edu
#endif