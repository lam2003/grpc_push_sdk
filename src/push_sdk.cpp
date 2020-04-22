#include <common/log.h>
#include <core/core.h>
#include <push_sdk.h>

#include <atomic>

std::atomic_flag once_flag = ATOMIC_FLAG_INIT;
volatile bool    registed  = false;

PushSDKRetCode SmsTransRegisterOnce(const char* logpath)
{
    PushSDKRetCode ret = PS_RET_SUCCESS;

    if (!once_flag.test_and_set()) {
        if ((ret = static_cast<PushSDKRetCode>(init_logger(logpath))) !=
            PS_RET_SUCCESS) {
            return ret;
        }
        registed = true;
    }

    while (!registed) {}

    return ret;
}

PushSDKRetCode
SmsTransUserLogin(PushSDKUserInfo info, PushSDKCallCB cb, void* data)
{
    return PS_RET_SUCCESS;
}

PushSDKRetCode
SmsTransUserLogout(PushSDKUserInfo user, PushSDKCallCB cb, void* data)
{
    return PS_RET_SUCCESS;
}

PushSDKRetCode SmsTransStart(uint32_t uid, uint64_t appid, uint64_t appkey)
{
    PushSDKRetCode ret;

    if ((ret = static_cast<PushSDKRetCode>(
             edu::PushSDK::Instance()->Initialize())) != PS_RET_SUCCESS) {
        return ret;
    }

    return PS_RET_SUCCESS;
}

PushSDKRetCode
SmsTransUserJoinGroup(PushSDKUserGroupInfo group, PushSDKCallCB cb, void* data)
{
    return PS_RET_SUCCESS;
}

PushSDKRetCode
SmsTransUserLeaveGroup(PushSDKUserGroupInfo group, PushSDKCallCB cb, void* data)
{
    return PS_RET_SUCCESS;
}

egc_handler_t SmsTransAddLinkStatusListener(SmsTransLinkstatus_Callback link_cb,
                                            void*                       arg)
{
    return 0;
}

egc_handler_t SmsTransAddUidMessageListener(const char* servicename,
                                            uint64_t    uid,
                                            SmsTransPushmsg_Callback msg_cb,
                                            void*                    arg)
{
    return 0;
}

egc_handler_t SmsTransAddGroupMessageListener(const char* servicename,
                                              uint64_t    grouptype,
                                              uint64_t    groupid,
                                              SmsTransPushmsg_Callback msg_cb,
                                              void*                    arg)
{
    return 0;
}

bool SmsTransRemoveGroupMessageListener(egc_handler_t id)
{
    return false;
}

bool SmsTransRemoveLinkStatusListener(egc_handler_t id)
{
    return false;
}

bool SmsTransRemoveUidMessageListener(egc_handler_t id)
{
    return false;
}
