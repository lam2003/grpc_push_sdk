#include <common/config.h>
#include <common/utils.h>
#include <core/client.h>
#include <core/stream.h>

#include <sstream>

#include <grpc++/grpc++.h>

#define HASH_HEADER_KEY "suid"
#define UID_HEADER_KEY "uid"

namespace edu {

Stream::Stream(std::shared_ptr<Client> client)
{
    client_ = client;

    ctx_ = std::unique_ptr<grpc::ClientContext>(new grpc::ClientContext);
    // 填入header
    ctx_->AddMetadata(HASH_HEADER_KEY, std::to_string(client->GetSUID()));
    ctx_->AddMetadata(UID_HEADER_KEY, std::to_string(client->GetUID()));

    push_data_   = std::unique_ptr<PushData>(new PushData);
    rw_          = nullptr;
    status_      = StreamStatus::WAIT_CONNECT;
    grpc_status_ = grpc::Status::OK;
}

Stream::~Stream()
{
    Destroy();
}

void Stream::Init()
{
    status_ = StreamStatus::WAIT_CONNECT;
    log_t("WAIT_CONNECT");

    rw_ = client_->stub->AsyncPushRegister(
        ctx_.get(), client_->cq.get(),
        reinterpret_cast<void*>(ClientEvent::CONNECTED));
}

void Stream::Process(ClientEvent event, bool ok)
{
    switch (event) {
        case ClientEvent::CONNECTED: {
            rw_->Read(push_data_.get(),
                      reinterpret_cast<void*>(ClientEvent::READ_DONE));
            status_ = StreamStatus::READY_TO_WRITE;

            if (ok) {
                log_t("CONNECTED");
                client_->on_connected();
            }
            break;
        }
        case ClientEvent::READ_DONE: {
            rw_->Read(push_data_.get(),
                      reinterpret_cast<void*>(ClientEvent::READ_DONE));

            log_t("READ_DONE");
            PushData*                 p = new PushData(*push_data_.get());
            std::shared_ptr<PushData> push_data(p);
            client_->on_read(push_data);

            break;
        }
        case ClientEvent::WRITE_DONE: {
            if (msg_queue_.empty()) {
                status_ = StreamStatus::READY_TO_WRITE;
                log_t("WRITE_DONE");
            }
            else {
                std::shared_ptr<PushRegReq> r = msg_queue_.front();
                rw_->Write(*r,
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
    msg_queue_.emplace_back(req);

    if (status_ == StreamStatus::READY_TO_WRITE) {
        std::shared_ptr<PushRegReq> r = msg_queue_.front();
        msg_queue_.pop_front();
        rw_->Write(*r, reinterpret_cast<void*>(ClientEvent::WRITE_DONE));
        status_ = StreamStatus::WAIT_WRITE_DONE;
    }
}

void Stream::SendMsgs(std::deque<std::shared_ptr<PushRegReq>>& msgs)
{
    for (auto it = msgs.begin(); it != msgs.end(); it++) {
        msg_queue_.emplace_back(*it);
    }
    msgs.clear();

    if (status_ == StreamStatus::READY_TO_WRITE) {
        if (msg_queue_.empty()) {
            return;
        }
        std::shared_ptr<PushRegReq> r = msg_queue_.front();
        msg_queue_.pop_front();
        rw_->Write(*r, reinterpret_cast<void*>(ClientEvent::WRITE_DONE));
        status_ = StreamStatus::WAIT_WRITE_DONE;
    }
}

void Stream::Destroy()
{
    ctx_         = nullptr;
    client_      = nullptr;
    push_data_   = nullptr;
    rw_          = nullptr;
    status_      = StreamStatus::WAIT_CONNECT;
    grpc_status_ = grpc::Status::OK;
}

bool Stream::IsConnected()
{
    if (status_ == StreamStatus::WAIT_CONNECT) {
        return false;
    }
    return true;
}

bool Stream::IsReadyToSend()
{
    if (status_ == StreamStatus::READY_TO_WRITE ||
        status_ == StreamStatus::WAIT_WRITE_DONE) {
        return true;
    }

    return false;
}

grpc::Status Stream::GrpcStatus()
{
    return grpc_status_;
}

void Stream::HalfClose()
{
    if (status_ == StreamStatus::WAIT_CONNECT ||
        status_ == StreamStatus::FINISHED ||
        status_ == StreamStatus::HALF_CLOSE) {
        return;
    }

    rw_->WritesDone(reinterpret_cast<void*>(ClientEvent::HALF_CLOSE));
    status_ = StreamStatus::HALF_CLOSE;
    log_t("HALF_CLOSE");
    return;
}

void Stream::Finish()
{
    if (status_ == StreamStatus::WAIT_CONNECT ||
        status_ == StreamStatus::FINISHED) {
        return;
    }

    rw_->Finish(&grpc_status_, reinterpret_cast<void*>(ClientEvent::FINISHED));
    status_ = StreamStatus::FINISHED;
    log_t("FINISHED");
}

}  // namespace edu