#ifndef EDU_PUSH_SDK_STREAM_H
#define EDU_PUSH_SDK_STREAM_H

#include <core/type.h>

#include <deque>

namespace edu {

class Client;

class Stream {
  public:
    Stream(std::shared_ptr<Client> client);
    virtual ~Stream();

  public:
    virtual void Init();
    virtual void Process(ClientEvent event);
    virtual void Finish();
    virtual void Send(std::shared_ptr<PushRegReq> req);
    virtual void SendMsgs(std::deque<std::shared_ptr<PushRegReq>>& msgs);
    virtual void Destroy();
    virtual bool IsConnected();
    virtual bool IsReadyToSend();
    virtual grpc::Status                GrpcStatus();
    virtual std::shared_ptr<PushRegReq> LastRequest();
    virtual void                        HalfClose();

  private:
    std::shared_ptr<Client>              client_;
    std::shared_ptr<grpc::ClientContext> ctx_;
    std::unique_ptr<PushData>            push_data_;
    std::unique_ptr<RW>                  rw_;
    StreamStatus                         status_;
    grpc::Status                         grpc_status_;
    std::shared_ptr<PushRegReq>          last_req_;

    std::mutex                              mux_;
    std::deque<std::shared_ptr<PushRegReq>> msg_queue_;
};

}  // namespace edu

#endif