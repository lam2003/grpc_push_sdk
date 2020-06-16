#ifndef EDU_PUSH_SDK_ASYNC_UPLOAD_H
#define EDU_PUSH_SDK_ASYNC_UPLOAD_H

#include <common/http_client.h>

#include <deque>
#include <thread>

namespace edu {
class ELKAsyncUploader {
  public:
    ELKAsyncUploader();

    ~ELKAsyncUploader();

  public:
  private:
    std::unique_ptr<std::thread> thread_;
    bool                         run_;
    std::mutex                   mux_;
};
}  // namespace edu

#endif