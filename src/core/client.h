#ifndef PUSH_SDK_CLIENT_H
#define PUSH_SDK_CLIENT_H

#include <core/type.h>

#include <atomic>
#include <memory>
#include <queue>
#include <string>
#include <thread>
#include <vector>


namespace edu {




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
    virtual void OnFinish(std::shared_ptr<PushRegReq> last_req,
                          grpc::Status                status)             = 0;
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

    virtual int  Initialize(uint32_t uid, uint64_t suid);
    virtual void Destroy();

    virtual void
    SetChannelStateListener(std::shared_ptr<ChannelStateListener> listener);

    virtual void
    SetClientStatusListener(std::shared_ptr<ClientStatusListener> listener);

    virtual void SetMessageHandler(std::shared_ptr<MessageHandler> hdl);

    virtual void Send(std::shared_ptr<PushRegReq> req);
    virtual void CleanQueue();

  private:
    void create_channel();
    void destroy_channel();
    void create_stream();
    void destroy_stream();

    void event_loop();
    void handle_event(StreamEvent event);
    void handle_cq_timeout();
    void check_channel_and_stream(bool ok);
    void try_to_send_ping();

    void check_and_notify_client_status_change(ClientStatus new_status);
    void check_and_notify_channel_state_change(ChannelState new_state);

  private:
    uint32_t                                uid_;
    uint64_t                                suid_;
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
    std::unique_ptr<RW>                 stream_;
    std::unique_ptr<grpc::ClientContext>    ctx_;
    std::unique_ptr<std::thread>            thread_;
    std::atomic<bool>                       run_;
    bool                                    init_;
    grpc::Status                            status_;
    std::shared_ptr<PushRegReq>             last_req_;
};
}  // namespace edu
#endif