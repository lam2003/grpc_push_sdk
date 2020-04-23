#include <common/log.h>
#include <core/core.h>
#include <push_sdk.h>

#include <atomic>
#include <mutex>

static std::mutex _mux;
volatile bool     _initialized = false;

PushSDKRetCode PushSDKInitialize(const char* log_dir)
{
    PushSDKRetCode               ret = PS_RET_SUCCESS;
    std::unique_lock<std::mutex> lock(_mux);

    if ((ret = static_cast<PushSDKRetCode>(init_logger(log_dir))) !=
        PS_RET_SUCCESS) {
        //日志库初始化失败, 不打日志
        return ret;
    }

    if ((ret = static_cast<PushSDKRetCode>(
             edu::PushSDK::Instance()->Initialize())) != PS_RET_SUCCESS) {
        log_e("push_sdk initailized failed. ret=%d", ret);
        return ret;
    }

    _initialized = true;
    return ret;
}

void PushSDKDestroy()
{
    std::unique_lock<std::mutex> lock(_mux);

    if (!_initialized) {
        return;
    }

    edu::PushSDK::Instance()->Destroy();
    _initialized = false;
}
