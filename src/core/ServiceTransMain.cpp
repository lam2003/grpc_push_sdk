#include "ServiceTransMain.h"
#include <iostream>
#include "EgcInit.h"
#include "EgcProtoHelper.h"
#include "EgcRpcClient.h"
#include "common/SLog.h"

namespace ServiceMeshTrans {

ServiceTransMain::ServiceTransMain()
  : stc_loginout_(stc_login_, stc_logout_), taskQueue_()
{
  EgcTrans::PinLibrary();
  //初始化grpc库
  g_gli_initializer.summon();
}

ServiceTransMain::~ServiceTransMain()
{
  unregiser_handler();
  EgcTrans::RPC::instance()->Stop();
  taskQueue_.quit();
  taskQueue_.wait();
  EgcTrans::UnpinLibrary();
  // spdlog::drop_all();
  std::cout << "after drop all " << std::endl;
}

/* credential generation */
bool ServiceTransMain::CreateCred(EgcTrans::CredOptions& options)
{
  return false;
}

SmsTransReturnCode ServiceTransMain::Start(uint64_t appid, uint64_t uid, uint64_t appKey, std::string pushsvr)
{
  appkey_ = appKey;
  appid_ = appid;
  SLOGI("ServiceTransMain start with appid {} appkey {} pushsvr {} uid {} ", appid, appKey, pushsvr, uid);
  EgcTrans::CredOptions opts;
  //创建成功后立即注册
  register_handler();
  return EgcTrans::RPC::instance()->Start(CreateCred(opts) ? &opts : nullptr, {{pushsvr}});
}

bool ServiceTransMain::PrepareLogin(EgcTrans::EgcUser user, SmsTransReturnCode& code)
{
  std::lock_guard<std::mutex> lk(uid_mtx_);
  auto itr = uidmap_.find(user.uid_);
  if (itr == uidmap_.end())
  {
    user_t info = std::make_shared<EgcTrans::EgcUser>(user);
    uidmap_[user.uid_] = info;
    SLOGI("user {} login ,insert to map ", user.uid_);
    code = SmsTransReturnCode::E_RETURN_SUCCESS;
    return true;
  }
  //已经登陆
  code = SmsTransReturnCode::E_RETURN_ALREADYLOGINED;
  return false;
}

bool ServiceTransMain::PrepareLogout(EgcTrans::EgcUser user,
                                     SmsTransReturnCode& code)
{
  std::lock_guard<std::mutex> lk(uid_mtx_);
  auto itr = uidmap_.find(user.uid_);
  if (itr == uidmap_.end())
  {
    code = E_RETURN_NOTLOGINED;
    return false;
  }
  //已经登陆，合法
  code = E_RETURN_SUCCESS;
  return true;
}

SmsTransReturnCode ServiceTransMain::UserLogin(SmsTransUserInfo info)
{
  // login或者logout 至少有一个正在处理中
  TRUE_RETURN_RESULT(stc_loginout_.isWork(), E_RETURN_CALL_MUTEX);
  //处理login中
  stc_loginout_.setWork1();
  if (info.uid == 0)
  {
    //参数错误
    return E_RETURN_TRANS_ERR_ARG;
  }
  //记录UID
  EgcTrans::EgcParam::instance()->SetUid(info.uid);
  //登陆时，收集用户信息
  static EgcTrans::EgcUser user;
  user.appid_ = appid_;
  // user.appkey_ = appkey_;
  user.account_ = std::string(info.account, info.account_size);
  user.passwd_ = std::string(info.passwd, info.passwd_size);
  user.token_ = std::string(info.token, info.token_size);
  user.uid_ = info.uid;
  SmsTransReturnCode code;
  bool ret = PrepareLogin(user, code);
  if (!ret)
  {
    return code;
  }
  ret = ProcessLogin(user);
  if (!ret)
  {
    return E_RETURN_REQUEST_ERR;
  }
  return E_RETURN_SUCCESS;
}

bool ServiceTransMain::CleanUid(std::string caller, EgcTrans::EgcUser user)
{
  std::lock_guard<std::mutex> lk(uid_mtx_);
  auto itr = uidmap_.find(user.uid_);
  if (itr == uidmap_.end())
  {
    SLOGW("caller {} ==not find {} cannot clean uid info ", caller, user.uid_);
    return false;
  }
  SLOGW("caller {} ==find uid {} ==clean uid info ", caller, user.uid_);
  uidmap_.erase(user.uid_);
  return true;
}

void ServiceTransMain::onLoginFail(SmsTransCallErrType err,
                                   EgcTrans::EgcUser user)
{
  login_result_clk_.fail(err, EgcTrans::callerr2str(err).c_str());
  stc_loginout_.setDone1();
  CleanUid("onLoginFail =>", user);
}

std::string ServiceTransMain::genCtx()
{
  static uint64_t ctxnum = 0;
  return "_REQ_" + std::to_string(ctxnum++) + "_" +
    std::to_string(EgcCommon::getCurHaomiao());
}

bool ServiceTransMain::requestLogin_(EgcTrans::EgcUser user,
                                     EgcTrans::EvtCallback clk)
{
  SLOGW("requestLogin_");
  EgcTrans::IEgcInnerEvt ptr = std::make_shared<EgcTrans::EgcInternalEvent>();
  ptr->event_code = EgcTrans::EGC_INTERNAL_EVENT_LOGIN;
  ptr->login.uid_ = user.uid_;
  // token_
  std::copy(user.token_.begin(), user.token_.end(), ptr->login.token_);
  ptr->login.token_[user.token_.size()] = '\0';
  // account
  std::copy(user.account_.begin(), user.account_.end(), ptr->login.account_);
  ptr->login.account_[user.account_.size()] = '\0';
  // passwd
  std::copy(user.passwd_.begin(), user.passwd_.end(), ptr->login.passwd_);
  ptr->login.passwd_[user.passwd_.size()] = '\0';
  ptr->setCallback(clk);
  ptr->setCtx("login" + genCtx());
  bool ret = EgcTrans::RPC::instance()->PushRequest(ptr);
  return ret;
}

bool ServiceTransMain::ProcessLogin(EgcTrans::EgcUser user)
{
  bool ret = requestLogin_(
    user, [this, user](SmsTransCallErrType err, std::string ctx,
                       uint32_t errCode, std::string errDesc)
    {
      static EgcTrans::EgcUser s_user = user;
      SLOGW("UserRequestLogin callback:{} errCode {}",
            EgcTrans::callerr2str(err), errCode);
      if (err == CALL_OK)
      {
        this->login_result_clk_.success();
        this->stc_loginout_.setDone1();
        //成功才做保存
        EgcTrans::EgcParam::instance()->SetUser(user.account_, user.passwd_,
                                                user.token_);
      }
      else
      {
        this->onLoginFail(err, s_user);
      }
    });
  SLOGW("requestLogin_ RET  {}", ret);
  return ret;
}

/////////////////////////////////LOGOUT////////////////////
bool ServiceTransMain::requestLogout_(EgcTrans::EgcUser info,
                                      EgcTrans::EvtCallback clk)
{
  SLOGW("requestLogout_");
  EgcTrans::IEgcInnerEvt ptr = std::make_shared<EgcTrans::EgcInternalEvent>();
  ptr->event_code = EgcTrans::EGC_INTERNAL_EVENT_LOGOUT;
  // appkey直接从param获取
  ptr->logout.appid_ = info.appid_;
  ptr->logout.uid_ = info.uid_;
  ptr->setCtx("logout" + genCtx());
  ptr->setCallback(clk);
  bool ret = EgcTrans::RPC::instance()->PushRequest(ptr);
  return ret;
}

bool ServiceTransMain::ProcessLogout(EgcTrans::EgcUser user)
{
  bool ret = requestLogout_(
    user, [this, user](SmsTransCallErrType err, std::string ctx,
                       uint32_t errCode, std::string errDesc)
    {
      SLOGW("UserRequestLogout callback:{} errCode{} errDesc{}",
            EgcTrans::callerr2str(err), errCode, errDesc);
      if (err == CALL_OK)
      {
        this->logout_result_clk_.success();
        this->stc_loginout_.setDone2();
        CleanUid("logout ok=>", user);
      }
      else
      {
        this->logout_result_clk_.fail(err,
                                      EgcTrans::callerr2str(err).c_str());
        this->stc_loginout_.setDone2();
        CleanUid("logout fail or timeout ==>", user);
      }
    });
  return ret;
}

SmsTransReturnCode ServiceTransMain::UserLogout(SmsTransUserInfo info)
{
  TRUE_RETURN_RESULT(stc_loginout_.isWork(), E_RETURN_CALL_MUTEX);
  stc_loginout_.setWork2();
  EgcTrans::EgcUser user;
  user.account_ = std::string(info.account);
  user.passwd_ = std::string(info.passwd);
  user.token_ = std::string(info.token);
  user.uid_ = info.uid;
  SmsTransReturnCode code;
  bool ret = PrepareLogout(user, code);
  if (!ret)
  {
    return code;
  }
  ret = ProcessLogout(user);
  if (!ret)
  {
    return E_RETURN_REQUEST_ERR;
  }
  return E_RETURN_SUCCESS;
}

///////////////////
void ServiceTransMain::register_handler()
{
  EgcTrans::RPC::instance()->SetOnLinkStatus([this](SmsTransLinkStatus status)
  {
    bool ok = true;
    switch (status)
    {
    case SmsTransLinkStatus::E_LINK_SHUTDOWN:
      SLOGI("LINK E_LINK_SHUTDOWN");
      break;
    case SmsTransLinkStatus::E_LINK_CREATE_OK:
      SLOGI("LINK E_LINK_CREATE_OK");
      break;
    case SmsTransLinkStatus::E_LINK_CREATE_RETRY:
      SLOGI("LINK E_LINK_CREATE_RETRY");
      break;
    case SmsTransLinkStatus::E_LINK_FAILURE:
      SLOGI("LINK E_LINK_FAILURE");

      break;
    case SmsTransLinkStatus::E_LINK_LOGIN_OK:
      SLOGI("LINK E_LINK_LOGIN_OK");
      break;
    case SmsTransLinkStatus::E_LINK_LOGINTING:
      SLOGI("E_LINK_LOGINTING");
      break;
    default:
      SLOGI("LINK status {}", status);
      ok = false;
      break;
    }
    //一定要注意，main要在client之后销毁，保证this有效
    if (ok)
    {
      sslis_.Dispatch(status);
    }
  });
  EgcTrans::RPC::instance()->SetOnMessage([this](RecvPushMessage msg)
  {
    taskQueue_.post(true, [=]()
    {
      SLOGI("=====ON MSG: {} size:{}", msg.msgData.c_str(), msg.msgData.size());
      if (msg.msgtype == PUSH_MSG_BY_GROUP)
      {
        gmsglis_.Dispatch(msg.serviceName, msg.groupType, msg.groupId,
                          msg.msgData.c_str(), msg.msgData.size());
      }
      else if (msg.msgtype == PUSH_MSG_BY_UID)
      {
        umsglis_.Dispatch(msg.serviceName, msg.uid, msg.msgData.c_str(),
                          msg.msgData.size());
      }
    });
  });
}

void ServiceTransMain::unregiser_handler()
{
  EgcTrans::RPC::instance()->SetOnLinkStatus(nullptr);
  EgcTrans::RPC::instance()->SetOnMessage(nullptr);
}

///////////////////join group
SmsTransReturnCode ServiceTransMain::UserJoinGroup(SmsTransUserGroup group, SUC suc, ERR err,
                                                   void* data)
{
  std::string ctx = "join" + genCtx();
  optlis_.AddJoinOpt(ctx, suc, err, data);
  SLOGW("UserJoinGroup");
  EgcTrans::IEgcInnerEvt ptr = std::make_shared<EgcTrans::EgcInternalEvent>();
  ptr->event_code = EgcTrans::EGC_INTERNAL_EVENT_JOIN;
  ptr->setCtx(ctx);
  ptr->join.uid = EgcTrans::EgcParam::instance()->GetUid();
  ptr->join.userGroupId = group.userGroupId;
  ptr->join.userGroupType = group.userGroupType;
  ptr->setCallback([this](SmsTransCallErrType err, std::string ctx,
                          uint32_t errCode, std::string errDesc)
  {
    SLOGW("UserJoinGroup callback:{} errCode{} errDesc",
          EgcTrans::callerr2str(err), errCode, errDesc);
    auto ptr = this->optlis_.PopJoinOpt(ctx);
    if (ptr)
    {
      if (err == CALL_OK)
      {
        ptr->success();
      }
      else
      {
        ptr->fail(err, EgcTrans::callerr2str(err).c_str());
      }
    }
    else
    {
      SLOGE("PopJoinOpt fail ,not find leavegroup opt clk");
    }
  });
  bool ret = EgcTrans::RPC::instance()->PushRequest(ptr);
  if (!ret)
  {
    return E_RETURN_REQUEST_ERR;
  }
  return E_RETURN_SUCCESS;
}

// leave group
SmsTransReturnCode ServiceTransMain::UserLeaveGroup(SmsTransUserGroup group, SUC suc, ERR err,
                                                    void* data)
{
  std::string ctx = "leave" + genCtx();
  optlis_.AddLeaveOpt(ctx, suc, err, data);
  SLOGW("UserLeaveGroup");
  EgcTrans::IEgcInnerEvt ptr = std::make_shared<EgcTrans::EgcInternalEvent>();
  ptr->event_code = EgcTrans::EGC_INTERNAL_EVENT_LEAVE;
  ptr->setCtx(ctx);
  ptr->leave.uid = EgcTrans::EgcParam::instance()->GetUid();
  ptr->leave.userGroupId = group.userGroupId;
  ptr->leave.userGroupType = group.userGroupType;
  ptr->setCallback([this](SmsTransCallErrType err, std::string ctx,
                          uint32_t errCode, std::string errDesc)
  {
    SLOGW("UserLeaveGroup callback:{} errCode{} errDesc{}",
          EgcTrans::callerr2str(err), errCode, errDesc);
    auto ptr = this->optlis_.PopLeaveOpt(ctx);
    if (ptr)
    {
      if (err == CALL_OK)
      {
        ptr->success();
      }
      else
      {
        ptr->fail(err, EgcTrans::callerr2str(err).c_str());
      }
    }
    else
    {
      SLOGE("PopLeaveOpt fail, not find leavegroup opt clk");
    }
  });
  bool ret = EgcTrans::RPC::instance()->PushRequest(ptr);
  if (!ret)
  {
    return E_RETURN_REQUEST_ERR;
  }
  return E_RETURN_SUCCESS;
}


egc_handler_t ServiceTransMain::AddLinkStatusListener(
  SmsTrans_Linkstatus_Callback link_cb, void* arg)
{
  egc_handler_t id = sslis_.GenId();
  sslis_.AddSSLis(id, link_cb, arg);
  return id;
}

bool ServiceTransMain::RemoveLinkStatusListener(egc_handler_t id)
{
  return sslis_.RemoveSSLis(id);
}

egc_handler_t ServiceTransMain::AddUidMessageListener(
  std::string servicename, uint64_t uid, SmsTrans_Pushmsg_Callback msg_cb,
  void* arg)
{
  egc_handler_t id = umsglis_.GenId();
  umsglis_.Add(id, servicename, uid, msg_cb, arg);
  return id;
}

bool ServiceTransMain::RemoveUidMessageListener(egc_handler_t id)
{
  return umsglis_.Remove(id);
}

egc_handler_t ServiceTransMain::AddGroupMessageListener(
  std::string servicename, uint64_t grouptype, uint64_t groupid,
  SmsTrans_Pushmsg_Callback msg_cb, void* arg)
{
  egc_handler_t id = gmsglis_.GenId();
  gmsglis_.Add(id, servicename, grouptype, groupid, msg_cb, arg);
  return id;
}

bool ServiceTransMain::RemoveGroupMessageListener(egc_handler_t id)
{
  return gmsglis_.Remove(id);
}
} // namespace ServiceMeshTrans
