#include "ServiceTransPushIf.h"
#include <iostream>
#include "EgcInit.h"
#include "ServiceTransMain.h"
#include "common/SLog.h"
// #include "resolver/EgcResolver.h"


SmsTransReturnCode SmsTrans_RegisterOnce(const char* logpath)
{
  // bool ret = EduPcResolver::RegisterOnce(logpath);


  std::string logpathstr = std::string(logpath);
  try
  {
    //先初始化日志库  
    EgcCommon::SLog::Init(logpathstr);
  }
  catch (std::exception* e)
  {
    std::cout << "RegisterOnce init log exception " << e->what() << std::endl;
    return E_RETURN_INITLOG_FAIL;
  }
  SLOGI("RegisterOnce with logpath ok {}", logpathstr);
  EgcTrans::ConfigureGrpc();
  // if (!ret)
  //   SLOGI("EduPcResolver with logpath fail {}", logpathstr);
  return E_RETURN_SUCCESS;
}

SmsTransReturnCode SmsTrans_UserLogin(SmsTransUserInfo info, SmsTrans_SucCallback suc, SmsTrans_ErrCallback err,
                                      void* data)
{
  ServiceMeshTrans::ServiceTransMain::instance()->login_result_clk_.set(suc, err, data,
                                                                        "USER_LOGIN_CLK");
  return ServiceMeshTrans::ServiceTransMain::instance()->UserLogin(info);
}

SmsTransReturnCode SmsTrans_UserLogout(SmsTransUserInfo user, SmsTrans_SucCallback suc, SmsTrans_ErrCallback err,
                                       void* data)
{
  ServiceMeshTrans::ServiceTransMain::instance()->logout_result_clk_.set(suc, err, data,
                                                                         "USER_LOGOUT_CLK");
  return ServiceMeshTrans::ServiceTransMain::instance()->UserLogout(user);
}

SmsTransReturnCode SmsTrans_Start(egc_uid_t uid, uint64_t appid, uint64_t appkey)
{
  //采用默认值
  if (appid == 0 || appkey == 0)
  {
    EgcTrans::EgcParam::instance()->SetAppid(ENUM_APP_SESSION);
    EgcTrans::EgcParam::instance()->SetAppKey(appkey);
  }
  if (uid == 0)
  {
    return E_RETURN_UID_ZERO;
  }
  //计算suid
  uint64_t suid = EgcTrans::genSuid(uid, uint16_t(EgcTrans::getProtoTT()));
  EgcTrans::EgcParam::instance()->SetUid(uid);
  EgcTrans::EgcParam::instance()->SetSUid(suid);
  return ServiceMeshTrans::ServiceTransMain::instance()->Start(ENUM_APP_SESSION, uid, appkey,
                                                               EgcTrans::EgcParam::instance()->GetPushSvr());
}

SmsTransReturnCode SmsTrans_UserJoinGroup(SmsTransUserGroup group, SmsTrans_SucCallback suc,
                                          SmsTrans_ErrCallback err, void* data)
{
  return ServiceMeshTrans::ServiceTransMain::instance()->UserJoinGroup(group, suc, err, data);
}

SmsTransReturnCode SmsTrans_UserLeaveGroup(SmsTransUserGroup group,
                                           SmsTrans_SucCallback suc, SmsTrans_ErrCallback err, void* data)
{
  return ServiceMeshTrans::ServiceTransMain::instance()->UserLeaveGroup(group, suc, err, data);
}

egc_handler_t SmsTrans_AddLinkStatusListener(SmsTrans_Linkstatus_Callback link_cb, void* arg)
{
  return ServiceMeshTrans::ServiceTransMain::instance()->AddLinkStatusListener(link_cb, arg);
}

egc_handler_t SmsTrans_AddUidMessageListener(std::string servicename, uint64_t uid,
                                         SmsTrans_Pushmsg_Callback msg_cb, void* arg)
{
  return ServiceMeshTrans::ServiceTransMain::instance()->AddUidMessageListener(servicename, uid,
                                                                               msg_cb, arg);
}

egc_handler_t SmsTrans_AddGroupMessageListener(std::string servicename, uint64_t grouptype,
                                           uint64_t groupid,
                                           SmsTrans_Pushmsg_Callback msg_cb, void* arg)
{
  return ServiceMeshTrans::ServiceTransMain::instance()->AddGroupMessageListener(
    servicename, grouptype, groupid, msg_cb, arg);
}

bool SmsTrans_RemoveGroupMessageListener(egc_handler_t id)
{
  return ServiceMeshTrans::ServiceTransMain::instance()->RemoveGroupMessageListener(id);
}

bool SmsTrans_RemoveLinkStatusListener(egc_handler_t id)
{
  return ServiceMeshTrans::ServiceTransMain::instance()->RemoveLinkStatusListener(id);
}

bool SmsTrans_RemoveUidMessageListener(egc_handler_t id)
{
  return ServiceMeshTrans::ServiceTransMain::instance()->RemoveUidMessageListener(id);
}
