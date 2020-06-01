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

class Stream;

class ChannelStateListener {
  public:
    ChannelStateListener() {}
    virtual ~ChannelStateListener() {}

  public:
    virtual void NotifyChannelState(ChannelState state) = 0;
};

class ClientStatusListener {
  public:
    ClientStatusListener() {}
    ~ClientStatusListener() {}

  public:
    virtual void OnClientStatusChange(StreamStatus statue) = 0;
    virtual void OnFinish(std::shared_ptr<PushRegReq> last_req,
                          grpc::Status                status)             = 0;
};

class MessageHandler {
  public:
  public:
    virtual void OnMessage(std::shared_ptr<PushData> msg) = 0;
};

class Client : public std::enable_shared_from_this<Client> {
    friend class Stream;

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
    void on_read(std::shared_ptr<PushData> push_data);
    void on_connected();
    void create_and_init_stream();
    void create_channel_and_stub();
    void check_and_notify_channel_state();
    void check_and_reconnect();
    void send_all_msgs();

  public:
    std::unique_ptr<grpc::CompletionQueue> cq;
    std::unique_ptr<Stub>                  stub;
    std::shared_ptr<grpc::Channel>         channel;

  private:
    std::unique_ptr<Stream>               st_;
    bool                                  init_;
    std::unique_ptr<std::thread>          thread_;
    std::atomic<bool>                     run_;
    std::shared_ptr<ChannelStateListener> channel_state_lis_;

    ChannelState last_channel_state_;
    int64_t      last_heartbeat_ts_;
    uint32_t     uid_;
    uint64_t     suid_;

    std::deque<std::shared_ptr<PushRegReq>> msg_queue_;
    std::mutex                              mux_;

    static std::atomic<uint32_t> port_index_;
};
}  // namespace edu
#endif