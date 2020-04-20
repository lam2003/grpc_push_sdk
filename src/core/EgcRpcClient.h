#pragma once
#include "EgcRpcDelegate.h"
#include "EgcRpcStream.h"
#include "common/EgcSingleton.h"
namespace EgcTrans {

class RpcClient : public IClientDelegate {
public:
  RpcClient();
  ~RpcClient();
  void SetOnMessage(onMessage cb) override;
  void SetOnLinkStatus(onLinkStatus) override;

  bool OnOpenStream(/*egc_result_t r, const char* p, egc_size_t len*/) override;
  egc_time_t OnCloseStream(int reconnect_attempt) override;


  SmsTransReturnCode Start(EgcTrans::CredOptions *opts,
                                      const std::vector<std::string>& ips) const;
  bool PushRequest(EgcTrans::IEgcInnerEvt evt) const;

  bool Stop();
private:
  RpcStream* stream_io_;
  EgcChannel* egcChannel_;

};

typedef EgcCommon::SingleTon<RpcClient> RPC;
} // namespace EgcTrans
