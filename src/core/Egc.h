#pragma once
#include <cstdint>
#include <string>
#include <functional>
#include "ServiceTransPushIf.h"
#include "EgcConsts.h"
#include "common/BuildConfig.h"


#if defined(OS_LINUX)
#include <arpa/inet.h>
#endif

#if defined(_Atomic)
#include <stdatomic.h>
#define ATOMIC_INT std::atomic_int
#define ATOMIC_UINT64 std::atomic_ullong
#else
#include <atomic>
#define ATOMIC_INT std::atomic<int>
#define ATOMIC_UINT64 std::atomic<unsigned long long>
#endif

typedef uint64_t egc_time_t; //nano seconds timestamp
typedef uint32_t egc_size_t;
typedef int egc_result_t;


typedef enum {
  I_LINK_CONNECTING = 1,
  I_LINK_LOGINTING = 2,
  I_LINK_READY = 3,
  I_LINK_FAILURE = 4,
  I_LINK_SHUTDOWN = 5,
  I_LINK_CQ_ERR = 6,
  I_LINK_RE_STREAM_CONNECTING = 7,
  //重新建立流
  I_LINK_RE_STREAM_OK_NEED_LOGIN = 8,
  //重新建立流成功，需要重新登录

} INNER_LinkStatus;

using SUC = SmsTrans_SucCallback;
using ERR = SmsTrans_ErrCallback;

using onWriteReady = std::function<void(void*)>;
using onLinkStatus = std::function<void(SmsTransLinkStatus)>;

struct RecvPushMessage {
  SmsTransPushMsgType msgtype;
  uint64_t suid;
  uint32_t uid;
  uint64_t groupType;
  uint64_t groupId;
  std::string serviceName;
  std::string msgData;
  uint64_t seqNum;
  std::string serverId;
};

using onMessage = std::function<void(RecvPushMessage)>;

namespace EgcTrans {

struct EgcUser {
  uint64_t appid_;
  //std::string appkey_;
  uint64_t uid_;
  std::string token_;
  std::string account_;
  std::string passwd_;
};

std::string callerr2str(SmsTransCallErrType err);
using EvtCallback = std::function<void(SmsTransCallErrType, std::string ctx, 
  uint32_t errCode, std::string errDesc)>;


}
