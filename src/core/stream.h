#ifndef EDU_PUSH_SDK_STREAM_H
#define EDU_PUSH_SDK_STREAM_H

#include <core/type.h>

#include <deque>

namespace edu {

class Client;

class StreamEeventListener {
  public:
    StreamEeventListener();
    virtual ~StreamEeventListener();

  public:
    virtual void OnConnected()                               = 0;
    virtual void OnRead(std::shared_ptr<PushData> push_data) = 0;
};

class Stream {
  public:
    Stream(std::shared_ptr<Client> client);
    virtual ~Stream();

  public:
    virtual void Init();
    virtual void Process(ClientEvent event);
    virtual void Finish();
    virtual void Send(std::shared_ptr<PushRegReq> req);

  private:
    std::shared_ptr<Client>               client_;
    std::shared_ptr<grpc::ClientContext>  ctx_;
    std::shared_ptr<grpc::Channel>        channel_;
    std::unique_ptr<Stub>                 stub_;
    std::unique_ptr<PushData>             push_data_;
    std::shared_ptr<StreamEeventListener> listener_;
    std::unique_ptr<RW>                   rw_;

    StreamStatus status_;
    bool         need_to_finish_;
    int64_t      last_heart_beat_ts_;

    std::mutex                              mux_;
    std::deque<std::shared_ptr<PushRegReq>> msg_queue_;
    grpc::Status                            grpc_status_;

    static std::atomic<uint32_t> port_index_;
};

}  // namespace edu

#endif