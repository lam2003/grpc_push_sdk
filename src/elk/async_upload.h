#ifndef EDU_PUSH_SDK_ASYNC_UPLOAD_H
#define EDU_PUSH_SDK_ASYNC_UPLOAD_H

#include <common/http_client.h>
#include <common/singleton.h>
#include <common/utils.h>
#include <elk/upload_request.h>

#include <condition_variable>
#include <deque>
#include <thread>

namespace edu {
class ELKAsyncUploader : public Singleton<ELKAsyncUploader> {
    friend class Singleton<ELKAsyncUploader>;

  public:
    ~ELKAsyncUploader();

    void Initialize();

  public:
    ELKAsyncUploader();

    template <typename... Args> void Push(Args... args)
    {
        if (!run_) {
            return;
        }
        std::unique_lock<std::mutex> mux_;
        queue_.emplace_back(
            std::make_shared<ELKUploadItem>(std::forward<Args>(args)...));
        if (queue_.size() >
            static_cast<size_t>(Config::Instance()->elk_upload_min_size)) {
            cond_.notify_one();
        }
    }

  private:
    std::unique_ptr<HttpClient>                http_client_;
    std::unique_ptr<std::thread>               thread_;
    bool                                       run_;
    std::mutex                                 mux_;
    std::deque<std::shared_ptr<ELKUploadItem>> queue_;
    std::condition_variable                    cond_;
};
}  // namespace edu

#define ELK_UPLOAD(...)                                                        \
    ELKAsyncUploader::Instance()->Push(                                        \
        Utils::GetSystemTime("%Y-%m-%d %H:%M:%S"), __VA_ARGS__)

#endif