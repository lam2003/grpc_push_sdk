#pragma once

#include <stdint.h>
#include "common/Singleton.h"
#include "Egc.h"

//for test domain
#define TEST0  "183.36.117.51:15000"
#define TEST1  "58.215.170.217:15000"
#define TEST2  "edu100dns:///front.100.com:15000"
#define TEST3  "front.100.com:15000"
#ifdef _DEBUG_SVR
#define PUSH_SVR_DOMAIN TEST3
#else
#define PUSH_SVR_DOMAIN "front.100.com:15000"
#endif
#define EGC_CONSOLE_LOG_NAME "SDK"
#define EGC_FILE_LOG_NAME "MESH"

static std::string GRPC_LOG_NAME = "edu_grpc_log";

//修复io释放
#define ONE_CHANNEL 0
//退出cq和grpc
#define USE_SEND_SHUTDOWN 1

// for test
#define HB_TEST_BY_SEND_THREAD 0
#define HB_TEST_AFTER_2_SECONDS 0

// for test only,online must set to 0
// 1 to disable heartbeat,online must set to 0
#define DISABLE_SEND_HB 0

// for test
#define UID 1234567890
#define GroupType 1
#define GroupId 888
#define AppKey 123456
#define ACCOUNT "qwerty"
#define PASSWD "abcdefg"
#define COOKIE_BASE64 \
  "sxi1nfclF1TS2WDzERt213TmctFtY9WDGB2LSSRZL1lkpSpEhwpUqFO6ZGDYSZ54"

#define PUSH_SVR_HEARTBEAT_HAOMIAO (10000)

constexpr int kConnectionTimeoutInHaomiao = 2000;
#define RPC_CALL_TIMEOUT (2000)
#define RPC_CALL_CHECK_TIMEOUT (2000)


// copy from packages/include/proto/ap/papabse.h
enum EAppMainType { TEST = 0, IM = 1, PUSHAP = 2, SESSION = 3, SERVICE = 4 };
enum EAppID {
  ENUM_APP_TEST = (1 << 8 | TEST),        // test
  ENUM_APP_IM = (1 << 8 | IM),            // yyim
  ENUM_APP_PUSHAP = (1 << 8 | PUSHAP),    // push
  ENUM_APP_SESSION = (1 << 8 | SESSION),  // yysession
  ENUM_APP_SERVICE = (1 << 8 | SERVICE)
};

namespace EgcTrans {
class EgcParam : public EgcCommon::SingleTon<EgcParam> {
 public:
  void SetAppKey(uint64_t key) { appkey_ = key; }
  uint64_t GetAppKey() { return appkey_; }
  void SetAppid(uint64_t appid) { appid_ = appid; }
  uint64_t GetAppid() { return appid_; }
  void SetUid(egc_uid_t uid) { uid_ = uid; }
  void SetSUid(uint64_t suid) { suid_ = suid; }
  egc_uid_t GetUid() { return uid_; }
  uint64_t GetSUid() { return suid_; }
  void SetUser(std::string accout, std::string passwd, std::string token) {
    passwd_ = passwd;
    accout_ = accout;
    token_ = token;
  }
  std::string GetUserAccout() { return accout_; }
  std::string GetUserPasswd() { return passwd_; }
  std::string GetToken() { return token_; }
  std::string GetPushSvr() { return PUSH_SVR_DOMAIN; }
 private:
  uint64_t appkey_;  // 欢聚云应用标识
  uint64_t appid_;
  egc_uid_t uid_;
  uint64_t suid_;
  // TODO 要替换为wstring么？
  std::string passwd_;
  std::string accout_;
  std::string token_;
};
}  // namespace EgcTrans