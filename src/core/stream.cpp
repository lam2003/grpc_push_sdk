#include <common/config.h>
#include <common/utils.h>
#include <core/client.h>
#include <core/stream.h>

#include <sstream>

#include <grpc++/grpc++.h>

namespace edu {

Stream::Stream(std::shared_ptr<Client> client)
{
    client_    = client;
    ctx_       = std::unique_ptr<grpc::ClientContext>(new grpc::ClientContext);
    push_data_ = std::unique_ptr<PushData>(new PushData);
    rw_        = nullptr;
    status_    = StreamStatus::WAIT_CONNECT;
    grpc_status_ = grpc::Status::OK;
    last_req_    = nullptr;
}

Stream::~Stream()
{
    Destroy();
}

void Stream::Init()
{
    status_ = StreamStatus::WAIT_CONNECT;
    log_t("stream status change to WAIT_CONNECT");

    rw_ = client_->stub->AsyncPushRegister(
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
            log_t("stream status change to READY_TO_WRITE");
            lock.unlock();

            client_->on_connected();

            break;
        }
        case ClientEvent::READ_DONE: {
            rw_->Read(push_data_.get(),
                      reinterpret_cast<void*>(ClientEvent::READ_DONE));
            lock.unlock();

            PushData*                 p = new PushData(*push_data_.get());
            std::shared_ptr<PushData> push_data(p);
            client_->on_read(push_data);

            break;
        }
        case ClientEvent::WRITE_DONE: {
            if (msg_queue_.empty()) {
                status_ = StreamStatus::READY_TO_WRITE;
                log_t("stream status change to READY_TO_WRITE");
            }
            else {
                std::shared_ptr<PushRegReq> r = msg_queue_.front();
                last_req_                     = r;
                rw_->Write(*r,
                           reinterpret_cast<void*>(ClientEvent::WRITE_DONE));
                status_ = StreamStatus::WAIT_WRITE_DONE;
                log_t("stream status change to WAIT_WRITE_DONE");
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

    msg_queue_.emplace_back(req);

    if (status_ == StreamStatus::READY_TO_WRITE) {
        std::shared_ptr<PushRegReq> r = msg_queue_.front();
        msg_queue_.pop_front();
        last_req_ = r;
        rw_->Write(*r, reinterpret_cast<void*>(ClientEvent::WRITE_DONE));
        status_ = StreamStatus::WAIT_WRITE_DONE;
        log_t("stream status change to WAIT_WRITE_DONE");
    }
}

void Stream::SendMsgs(std::deque<std::shared_ptr<PushRegReq>>& msgs)
{
    std::unique_lock<std::mutex> lock(mux_);

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
        last_req_ = r;
        rw_->Write(*r, reinterpret_cast<void*>(ClientEvent::WRITE_DONE));
        status_ = StreamStatus::WAIT_WRITE_DONE;
        log_t("stream status change to WAIT_WRITE_DONE");
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
    last_req_    = nullptr;
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

std::shared_ptr<PushRegReq> Stream::LastRequest()
{
    return last_req_;
}

void Stream::HalfClose()
{
    std::unique_lock<std::mutex> lock(mux_);

    if (status_ == StreamStatus::WAIT_CONNECT ||
        status_ == StreamStatus::FINISHED ||
        status_ == StreamStatus::HALF_CLOSE) {
        return;
    }

    rw_->WritesDone(reinterpret_cast<void*>(ClientEvent::HALF_CLOSE));
    status_ = StreamStatus::HALF_CLOSE;
    log_t("stream status change to HALF_CLOSE");
}

void Stream::Finish()
{
    std::unique_lock<std::mutex> lock(mux_);

    if (status_ == StreamStatus::WAIT_CONNECT ||
        status_ == StreamStatus::FINISHED) {
        return;
    }

    rw_->Finish(&grpc_status_, reinterpret_cast<void*>(ClientEvent::FINISHED));
    status_ = StreamStatus::FINISHED;
    log_t("stream status change to FINISHED");
}

}  // namespace edu