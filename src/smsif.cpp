#include <common/log.h>
#include <smsif.h>

SmsTransErrCode SmsTransRegisterOnce(const char* logpath)
{
    SmsTransErrCode ret = ERR_SMS_SUCCESS;

    if ((ret = static_cast<SmsTransErrCode>(edu::init_logger())) !=
        ERR_SMS_SUCCESS) {
        return ret;
    }

    return ret;
}

SmsTransErrCode SmsTransUserLogin(SmsTransUserInfo    info,
                                  SmsTransSucCallback suc,
                                  SmsTransErrCallback err,
                                  void*               data)
{
    return ERR_SMS_SUCCESS;
}

SmsTransErrCode SmsTransUserLogout(SmsTransUserInfo    user,
                                   SmsTransSucCallback suc,
                                   SmsTransErrCallback err,
                                   void*               data)
{
    return ERR_SMS_SUCCESS;
}

SmsTransErrCode SmsTransStart(egc_uid_t uid, uint64_t appid, uint64_t appkey)
{
    return ERR_SMS_SUCCESS;
}

SmsTransErrCode SmsTransUserJoinGroup(SmsTransUserGroup   group,
                                      SmsTransSucCallback suc,
                                      SmsTransErrCallback err,
                                      void*               data)
{
    return ERR_SMS_SUCCESS;
}

SmsTransErrCode SmsTransUserLeaveGroup(SmsTransUserGroup   group,
                                       SmsTransSucCallback suc,
                                       SmsTransErrCallback err,
                                       void*               data)
{
    return ERR_SMS_SUCCESS;
}

egc_handler_t SmsTransAddLinkStatusListener(SmsTransLinkstatus_Callback link_cb,
                                            void*                       arg)
{
    return 0;
}

egc_handler_t SmsTransAddUidMessageListener(std::string servicename,
                                            uint64_t    uid,
                                            SmsTransPushmsg_Callback msg_cb,
                                            void*                    arg)
{
    return 0;
}

egc_handler_t SmsTransAddGroupMessageListener(std::string servicename,
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
