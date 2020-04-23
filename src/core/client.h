#ifndef PUSH_SDK_CLIENT_H
#define PUSH_SDK_CLIENT_H

#include <proto/pushGateWay.grpc.pb.h>

#include <atomic>
#include <memory>
#include <thread>
#include <vector>

using PushGateway = grpc::push::gateway::PushGateway;
using Stub        = grpc::push::gateway::PushGateway::Stub;
using Stream =
    grpc::ClientAsyncReaderWriterInterface<grpc::push::gateway::PushRegReq,
                                           grpc::push::gateway::PushData>;
namespace edu {

enum class ClientEvent {
    CONNECTED  = 0x01,
    READ_DONE  = 0x02,
    WRITE_DONE = 0x03,
    FINISH     = 0x04
};

class Client {
  public:
    Client();
    virtual ~Client();

    virtual int  Initialize();
    virtual void Destroy();

  private:
    void  create_channel();
    void destroy_channel();
    void  create_stream();
    void destroy_stream();

    void event_loop();
    void handle_event(bool ok, ClientEvent event);
    int  handle_cq_timeout();
    void handle_cq_error();

  private:
    int                                    front_envoy_port_idx_;
    int64_t                                last_heartbeat_ts_;
    std::unique_ptr<grpc::CompletionQueue> cq_;
    std::shared_ptr<grpc::Channel>         channel_;
    std::unique_ptr<Stub>                  stub_;
    std::unique_ptr<Stream>                stream_;
    std::unique_ptr<grpc::ClientContext>   ctx_;
    std::unique_ptr<std::thread>           thread_;
    std::atomic<bool>                      run_;
};
}  // namespace edu
#endif