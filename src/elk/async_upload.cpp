#include <common/err_code.h>
#include <common/log.h>
#include <elk/async_upload.h>

namespace edu {

ELKAsyncUploader::ELKAsyncUploader()
{
    http_client_ = nullptr;
    thread_      = nullptr;
    run_         = false;
}

ELKAsyncUploader::~ELKAsyncUploader()
{
    if (run_) {
        {
            std::unique_lock<std::mutex> lock(mux_);
            run_ = false;
            cond_.notify_all();
        }
        thread_->join();
        thread_ = nullptr;

        http_client_->Close();
        http_client_ = nullptr;
    }
}

void ELKAsyncUploader::Initialize()
{
    if (run_) {
        return;
    }

    run_ = true;

    http_client_ = std::unique_ptr<HttpClient>(new HttpClient());
    http_client_->Initialize();

    thread_ = std::unique_ptr<std::thread>(new std::thread([this]() {
        ELKUploadRequest req;
        while (run_) {
            {
                std::unique_lock<std::mutex> lock(mux_);
                if (!queue_.empty()) {
                    std::swap(
                        const_cast<std::deque<std::shared_ptr<ELKUploadItem>>&>(
                            req.contents),
                        queue_);
                }
                else if (run_) {
                    cond_.wait_for(
                        lock, std::chrono::milliseconds(
                                  Config::Instance()->elk_upload_interval_ms));
                }
            }

            if (!req.contents.empty()) {
                std::string data = req;
                bool        ok   = false;
                do {
                    if ((ok = http_client_->Post(
                             Config::Instance()->elk_upload_host, 80,
                             Config::Instance()->elk_upload_path,
                             Config::Instance()->elk_upload_headers, data)) !=
                        true) {
                        //不丢弃数据,尝试重试
                        usleep(500 * 1000);
                    }
                } while (!ok && run_);
                req.contents.clear();
            }
        }
    }));
}

}  // namespace edu