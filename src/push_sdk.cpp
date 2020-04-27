#include <common/log.h>
#include <core/core.h>
#include <push_sdk.h>

#include <atomic>
#include <mutex>

static std::mutex        _mux;
static std::atomic<bool> _initialized(false);
static std::atomic<bool> _log_initialized(false);
static PushSDKCallCB     _cb_func = nullptr;
static void*             _cb_args = nullptr;

PushSDKRetCode PushSDKInitialize(uint32_t      uid,
                                 uint64_t      appid,
                                 uint64_t      appkey,
                                 const char*   log_dir,
                                 PushSDKCallCB cb_func,
                                 void*         cb_arg)
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

    if (!cb_func) {
        ret = PS_RET_CB_IS_NULL;
        log_e("call back function is null");
        return ret;
    }

    if ((ret = static_cast<PushSDKRetCode>(edu::PushSDK::Instance()->Initialize(
             uid, appid, appkey, cb_func, cb_arg))) != PS_RET_SUCCESS) {
        log_e("push_sdk initailize failed. ret={}", ret);
        return ret;
    }

    log_i("push_sdk initialize successfully. uid={}, appid={}, appkey={}", uid,
          appid, appkey);

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
    PushSDKRetCode               ret = PS_RET_SUCCESS;
    std::unique_lock<std::mutex> lock(_mux);

    if (!_initialized) {
        ret = PS_RET_SDK_UNINIT;
        return ret;
    }

    if (!user) {
        ret = PS_RET_USER_INFO_IS_NULL;
        log_e("login with user info(null) is not allow. ret={}", ret);
        return ret;
    }

    if ((ret = static_cast<PushSDKRetCode>(
             edu::PushSDK::Instance()->Login(*user))) != PS_RET_SUCCESS) {
        log_e("login failed. ret={}", ret);
        return ret;
    }

    return ret;
}

PushSDKRetCode PushSDKLogout()
{
    PushSDKRetCode ret = PS_RET_SUCCESS;
    if (!_initialized) {
        ret = PS_RET_SDK_UNINIT;
        return ret;
    }

    if ((ret = static_cast<PushSDKRetCode>(
             edu::PushSDK::Instance()->Logout())) != PS_RET_SUCCESS) {
        log_e("logout failed. ret={}", ret);
        return ret;
    }

    return ret;
}