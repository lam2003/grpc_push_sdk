#include <common/log.h>
#include <core/core.h>
#include <push_sdk.h>

#include <atomic>
#include <mutex>

static std::mutex _mux;
volatile bool     _initialized     = false;
volatile bool     _log_initialized = false;

PushSDKRetCode PushSDKInitialize(uint32_t uid, const char* log_dir)
{
    PushSDKRetCode               ret = PS_RET_SUCCESS;
    std::unique_lock<std::mutex> lock(_mux);

    if (_initialized) {
        log_w("push_sdk already initialized");
        return PS_RET_ALREADY_INIT;
    }

    if (!_log_initialized && (ret = static_cast<PushSDKRetCode>(
                                  init_logger(log_dir))) != PS_RET_SUCCESS) {
        //日志库初始化失败, 不打日志
        return ret;
    }
    _log_initialized = true;

    if ((ret = static_cast<PushSDKRetCode>(
             edu::PushSDK::Instance()->Initialize(uid))) != PS_RET_SUCCESS) {
        log_e("push_sdk initailize failed. ret={}", ret);
        return ret;
    }

    log_i("push_sdk initialize successfully. uid={}", uid);

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

PushSDKRetCode PushSDKLogin(PushSDKUserInfo* user)
{
    PushSDKRetCode ret = PS_RET_SUCCESS;

    if (!_initialized) {
        ret = PS_RET_SDK_UNINIT;
        return ret;
    }

    return ret;
}
