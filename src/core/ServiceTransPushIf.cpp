#include <ServiceTransPushIf.h>

SmsTransReturnCode SmsTrans_RegisterOnce(const char* logpath)
{
    return E_RETURN_SUCCESS;
}

SmsTransReturnCode SmsTrans_UserLogin(SmsTransUserInfo     info,
                                      SmsTrans_SucCallback suc,
                                      SmsTrans_ErrCallback err,
                                      void*                data)
{
    return E_RETURN_SUCCESS;
}

SmsTransReturnCode SmsTrans_UserLogout(SmsTransUserInfo     user,
                                       SmsTrans_SucCallback suc,
                                       SmsTrans_ErrCallback err,
                                       void*                data)
{
    return E_RETURN_SUCCESS;
}

SmsTransReturnCode
SmsTrans_Start(egc_uid_t uid, uint64_t appid, uint64_t appkey)
{
    return E_RETURN_SUCCESS;
}

SmsTransReturnCode SmsTrans_UserJoinGroup(SmsTransUserGroup    group,
                                          SmsTrans_SucCallback suc,
                                          SmsTrans_ErrCallback err,
                                          void*                data)
{
    return E_RETURN_SUCCESS;
}

SmsTransReturnCode SmsTrans_UserLeaveGroup(SmsTransUserGroup    group,
                                           SmsTrans_SucCallback suc,
                                           SmsTrans_ErrCallback err,
                                           void*                data)
{
    return E_RETURN_SUCCESS;
}

egc_handler_t
SmsTrans_AddLinkStatusListener(SmsTrans_Linkstatus_Callback link_cb, void* arg)
{
    return 0;
}

egc_handler_t SmsTrans_AddUidMessageListener(std::string servicename,
                                             uint64_t    uid,
                                             SmsTrans_Pushmsg_Callback msg_cb,
                                             void*                     arg)
{
    return 0;
}

egc_handler_t SmsTrans_AddGroupMessageListener(std::string servicename,
                                               uint64_t    grouptype,
                                               uint64_t    groupid,
                                               SmsTrans_Pushmsg_Callback msg_cb,
                                               void*                     arg)
{
    return 0;
}

bool SmsTrans_RemoveGroupMessageListener(egc_handler_t id)
{
    return false;
}

bool SmsTrans_RemoveLinkStatusListener(egc_handler_t id)
{
    return false;
}

bool SmsTrans_RemoveUidMessageListener(egc_handler_t id)
{
    return false;
}
