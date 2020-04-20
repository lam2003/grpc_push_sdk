#pragma once
#include <atomic>
#include <map>
#include "Egc.h"
#include "EgcCallback.h"
#include "EgcListener.h"
#include "EgcRpcClient.h"
#include "common/CheckStatus.h"
#include "common/Singleton.h"
#include "common/TaskQueue.h"

namespace ServiceMeshTrans {
class ServiceTransMain : public EgcCommon::SingleTon<ServiceTransMain> {
  friend class EgcCommon::SingleTon<ServiceTransMain>;

public:
  ServiceTransMain();
  ~ServiceTransMain();
  SmsTransReturnCode Start(uint64_t appid, uint64_t uid, uint64_t appKey,
                           /*std::string log, */ std::string pushsvr);
  SmsTransReturnCode UserLogin(SmsTransUserInfo info);
  SmsTransReturnCode UserLogout(SmsTransUserInfo info);
  SmsTransReturnCode UserJoinGroup(SmsTransUserGroup group, SUC suc, ERR err, void* data);
  SmsTransReturnCode UserLeaveGroup(SmsTransUserGroup group, SUC suc, ERR err, void* data);
  egc_handler_t AddLinkStatusListener(SmsTrans_Linkstatus_Callback link_cb,
                                  void* arg);
  bool RemoveLinkStatusListener(egc_handler_t id);
  egc_handler_t AddGroupMessageListener(std::string servicename, uint64_t grouptype,
                                    uint64_t groupid,
                                    SmsTrans_Pushmsg_Callback msg_cb, void* arg);
  bool RemoveGroupMessageListener(egc_handler_t);
  egc_handler_t AddUidMessageListener(std::string servicename, uint64_t uid,
                                  SmsTrans_Pushmsg_Callback msg_cb, void* arg);
  bool RemoveUidMessageListener(egc_handler_t);

private:
  bool CreateCred(EgcTrans::CredOptions& options);
  std::string genCtx();
  bool requestLogout_(EgcTrans::EgcUser info, EgcTrans::EvtCallback clk);
  bool requestLogin_(EgcTrans::EgcUser user, EgcTrans::EvtCallback clk);
  bool PrepareLogin(EgcTrans::EgcUser user, SmsTransReturnCode& code);
  bool PrepareLogout(EgcTrans::EgcUser user, SmsTransReturnCode& code);
  bool ProcessLogin(EgcTrans::EgcUser user);
  void onLoginFail(SmsTransCallErrType err, EgcTrans::EgcUser user);
  bool CleanUid(std::string caller, EgcTrans::EgcUser user);
  bool ProcessLogout(EgcTrans::EgcUser user);
  void register_handler();
  void unregiser_handler();

public:
  EgcTrans::ResultCallback login_result_clk_;
  EgcTrans::ResultCallback logout_result_clk_;
  EgcCommon::StatusCheck stc_loginout_;

private:
  EgcCommon::TaskQueue taskQueue_;
  EgcTrans::GroupOptClkListener optlis_;
  EgcTrans::StatusListener sslis_;
  EgcTrans::GroupMsgListenr gmsglis_;
  EgcTrans::UidMsgListener umsglis_;
  //EgcTrans::RpcClient* client_;
  uint64_t appid_;
  uint64_t appkey_;
  using user_t = std::shared_ptr<EgcTrans::EgcUser>;
  using uid2userMap = std::map<uint32_t, user_t>;
  uid2userMap uidmap_;
  std::mutex uid_mtx_;
  EgcCommon::OptStatus stc_login_;
  EgcCommon::OptStatus stc_logout_;
  uint64_t suid_;
};
} // namespace ServiceMeshTrans
