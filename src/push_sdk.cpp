#include <common/log.h>
#include <core/core.h>
#include <push_sdk.h>

#include <atomic>
#include <mutex>

static std::mutex        _mux;
static std::atomic<bool> _initialized(false);
static std::atomic<bool> _log_initialized(false);

PushSDKRetCode PushSDKInitialize(uint32_t       uid,
                                 uint64_t       appid,
                                 uint64_t       appkey,
                                 const char*    log_dir,
                                 PushSDKEventCB cb_func,
                                 void*          cb_arg)
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

    flush_logger();
    
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
        if (ret != PS_RET_CALL_TIMEOUT) {
            log_e("login failed. ret={}", ret);
        }
        return ret;
    }

    return ret;
}

PushSDKRetCode PushSDKLogout()
{
    PushSDKRetCode               ret = PS_RET_SUCCESS;
    std::unique_lock<std::mutex> lock(_mux);

    if (!_initialized) {
        ret = PS_RET_SDK_UNINIT;
        return ret;
    }

    if ((ret = static_cast<PushSDKRetCode>(
             edu::PushSDK::Instance()->Logout())) != PS_RET_SUCCESS) {
        if (ret != PS_RET_CALL_TIMEOUT) {
            log_e("logout failed. ret={}", ret);
        }
        return ret;
    }

    return ret;
}

PushSDKRetCode PushSDKJoinGroup(PushSDKGroupInfo* group)
{
    PushSDKRetCode               ret = PS_RET_SUCCESS;
    std::unique_lock<std::mutex> lock(_mux);

    if (!_initialized) {
        ret = PS_RET_SDK_UNINIT;
        return ret;
    }

    if ((ret = static_cast<PushSDKRetCode>(
             edu::PushSDK::Instance()->JoinGroup(*group))) != PS_RET_SUCCESS) {
        if (ret != PS_RET_CALL_TIMEOUT) {
            log_e("join group failed. ret={}", ret);
        }
        return ret;
    }

    return ret;
}

PushSDKRetCode PushSDKLeaveGroup(PushSDKGroupInfo* group)
{
    PushSDKRetCode               ret = PS_RET_SUCCESS;
    std::unique_lock<std::mutex> lock(_mux);

    if (!_initialized) {
        ret = PS_RET_SDK_UNINIT;
        return ret;
    }

    if ((ret = static_cast<PushSDKRetCode>(
             edu::PushSDK::Instance()->LeaveGroup(*group))) != PS_RET_SUCCESS) {
        if (ret != PS_RET_CALL_TIMEOUT) {
            log_e("leave group failed. ret={}", ret);
        }
        return ret;
    }

    return ret;
}

void PushSDKGetError(char** desc, int* code)
{
    std::string s;
    int         c;

    std::unique_lock<std::mutex> lock(_mux);
    edu::PushSDK::Instance()->GetLastError(s, c);

    *desc = (char*)malloc(s.length() + 1);
    memcpy(*desc, s.c_str(), s.length());
    (*desc)[s.length()] = '\0';

    *code = c;
}

PS_HANDLER PushSDKCreateHandler()
{
    return edu::PushSDK::Instance()->CreateHandler();
}

void PushSDKDestroyHandler(PS_HANDLER handler)
{
    if (!handler) {
        return;
    }

    edu::PushSDK::Instance()->DestroyHandler(
        reinterpret_cast<edu::Handler*>(handler));
}

void PushSDKSetConnStateCB(PS_HANDLER handler, PushSDKConnStateCB state_cb)
{
    if (!handler || !state_cb) {
        return;
    }

    edu::PushSDK::Instance()->AddConnStateCBToHandler(
        reinterpret_cast<edu::Handler*>(handler), state_cb);
}

void PushSDKSetUserMsgCB(PS_HANDLER handler, PushSDKUserMsgCB msg_cb)
{
    if (!handler || !msg_cb) {
        return;
    }

    edu::PushSDK::Instance()->AddUserMsgCBToHandler(
        reinterpret_cast<edu::Handler*>(handler), msg_cb);
}

void PushSDKSetGroupMsgCB(PS_HANDLER handler, PushSDKGroupMsgCB msg_cb)
{
    if (!handler || !msg_cb) {
        return;
    }
    edu::PushSDK::Instance()->AddGroupMsgCBToHandler(
        reinterpret_cast<edu::Handler*>(handler), msg_cb);
}