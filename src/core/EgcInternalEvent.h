#pragma once
#include <string>
#include <deque>
#include <memory>
#include "Egc.h"
#include "common/EgcUtils.h"
#include "common/SLog.h"

namespace EgcTrans {
enum EnumEgcInternalEvents {
  EGC_INTERNAL_EVENT_NULL = 0,
  EGC_INTERNAL_EVENT_LOGIN = 1,
  EGC_INTERNAL_EVENT_LOGOUT = 2,
  EGC_INTERNAL_EVENT_JOIN = 3,
  EGC_INTERNAL_EVENT_LEAVE = 4,
  EGC_INTERNAL_EVENT_HEARTBEAT = 5,
};

std::string evt_code2str(int code);

struct EgcInternalEventBase {
  virtual ~EgcInternalEventBase()
  {
  }
};

struct EgcInternalEventLogin : public EgcInternalEventBase {
  uint64_t appid_;
  uint64_t uid_;
  // uint64_t appkey_;
  char token_[2048];
  char account_[128];
  char passwd_[128];
};

struct EgcInternalEventLogout : public EgcInternalEventBase {
  std::uint64_t uid_;
  uint64_t appid_;
};

struct EgcInterNalEventHeartBeat : public EgcInternalEventBase {
  std::uint64_t uid;
  std::uint64_t delta;
};

struct EgcInternalEventJoinGroup : public EgcInternalEventBase {
  uint64_t uid;
  uint64_t userGroupType;
  uint64_t userGroupId;
};

struct EgcInternalEventLeaveGroup : public EgcInternalEventBase {
  uint64_t uid;
  uint64_t userGroupType;
  uint64_t userGroupId;
};

struct EgcInternalEvent {
  EgcInternalEvent()
    : event_code(EGC_INTERNAL_EVENT_NULL),
      start_at_(EgcCommon::getCurHaomiao())
  {
  }

  virtual ~EgcInternalEvent() { evtclk_ = nullptr; }
  uint32_t event_code;
  std::string context_;
  uint64_t start_at_; //Ê±¼ä´Á
  EvtCallback evtclk_;
  void setCallback(EvtCallback evt) { evtclk_ = std::move(evt); }
  void setCtx(std::string ctx) { context_ = ctx; };
  std::string getCtx() { return context_; }

  union {
    EgcInternalEventLogin login;
    EgcInternalEventJoinGroup join;
    EgcInterNalEventHeartBeat heartbeat;
    EgcInternalEventLogout logout;
    EgcInternalEventLeaveGroup leave;
  };
};

using IEgcInnerEvt = std::shared_ptr<EgcInternalEvent>;
using EgcEvtQ = std::unique_ptr<std::deque<IEgcInnerEvt>>;
} // namespace EgcTrans
