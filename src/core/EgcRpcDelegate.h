#pragma once
#include <atomic>
#include <chrono>
#include <iostream>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include "EgcConsts.h"
#include "EgcRpcInc.h"
#include "EgcInternalEvent.h"
#include "EgcProtoHelper.h"
#include "MoodyCamel/concurrentqueue.h"
#include "common/Singleton.h"

namespace EgcTrans {



class IClientDelegate {
public:
  IClientDelegate()
  {
  }

  virtual ~IClientDelegate()
  {
  }

  onMessage on_message_ = {nullptr};
  onLinkStatus on_linkstatus_ = {nullptr};
  virtual void SetOnMessage(onMessage cb) = 0;
  virtual void SetOnLinkStatus(onLinkStatus cb) = 0;

  //virtual bool Ready() const = 0; 

  virtual bool OnOpenStream(/*egc_result_t r, const char* p, egc_size_t len*/) = 0;
  virtual egc_time_t OnCloseStream(int reconnect_attempt) = 0;
};
}
