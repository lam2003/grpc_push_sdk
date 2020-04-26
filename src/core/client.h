#ifndef PUSH_SDK_CLIENT_H
#define PUSH_SDK_CLIENT_H

#include <proto/pushGateWay.grpc.pb.h>

#include <atomic>
#include <memory>
#include <queue>
#include <string>
#include <thread>
#include <vector>

using PushGateway      = grpc::push::gateway::PushGateway;
using Stub             = grpc::push::gateway::PushGateway::Stub;
using PushRegReq       = grpc::push::gateway::PushRegReq;
using LoginRequest     = grpc::push::gateway::LoginRequest;
using LoginResponse    = grpc::push::gateway::LoginResponse;
using PushData         = grpc::push::gateway::PushData;
using StreamURI        = grpc::push::gateway::StreamURI;
using UserTerminalType = grpc::push::gateway::UserTerminalType;
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
    CONNECTED       = 101,
    READY_TO_WRITE  = 102,
    WAIT_WRITE_DONE = 103,
    FINISHED        = 104
};

enum class ChannelState { OK, NO_READY };

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

class MessageHandler {
  public:
  public:
    virtual void OnMessage(std::shared_ptr<PushData> msg) = 0;
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

    virtual void SetMessageHandler(std::shared_ptr<MessageHandler> hdl);

    virtual void Send(std::shared_ptr<PushRegReq> req);

  private:
    void create_channel();
    void destroy_channel();
    void create_stream();
    void destroy_stream();

    void event_loop();
    void handle_event(ClientEvent event);
    void handle_cq_timeout();
    void check_channel_and_stream(bool ok);

    void check_and_notify_client_status_change(ClientStatus new_status);
    void check_and_notify_channel_state_change(ChannelState new_state);

  private:
    std::string                             uid_;
    int                                     front_envoy_port_idx_;
    int64_t                                 last_heartbeat_ts_;
    ClientStatus                            client_status_;
    ChannelState                            channel_state_;
    std::shared_ptr<ChannelStateListener>   state_listener_;
    std::shared_ptr<ClientStatusListener>   status_listener_;
    std::shared_ptr<MessageHandler>         msg_hdl_;
    std::shared_ptr<PushData>               msg_cache_;
    std::queue<std::shared_ptr<PushRegReq>> queue_;
    std::mutex                              mux_;
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