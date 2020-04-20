#include "EgcRpcClient.h"
#include "EgcRpcInc.h"
#include "common/SLog.h"

namespace EgcTrans {

RpcClient::RpcClient()
{
  egcChannel_ = new EgcChannel();
  if (egcChannel_)
  {
    stream_io_ = new RpcStream(this);
    if (stream_io_)
    {
      //创建stub
      stream_io_->SetChannel(egcChannel_);
      SLOGI("RpcStream create success ");
    }
    else
    {
      SLOGE("RpcStream create Fail ");
    }
  }else
  {
    SLOGE("EgcChannel create Fail ");
  }
}

RpcClient::~RpcClient()
{
  SLOGW("RpcClient Destroy ");
  Stop();
  SLOGW("RpcClient Destroy finished");
}

bool RpcClient::OnOpenStream(/*egc_result_t r, const char* p, egc_size_t len*/)
{
  return true;
}

egc_time_t RpcClient::OnCloseStream(int reconnect_attempt)
{
  return 0;
}

bool RpcClient::Stop()
{
  if (stream_io_)
  {
    stream_io_->ShutdownCQ();
    delete stream_io_;
    stream_io_ = nullptr;
  }
  else
  {
    SLOGE("error stream_io_ is null ，cannot stop ");
  }
  if (egcChannel_)
  {
    delete egcChannel_;
    egcChannel_ = nullptr;
  }
  return true;
}

bool RpcClient::PushRequest(EgcTrans::IEgcInnerEvt evt) const
{
  if (stream_io_)
  {
    return stream_io_->PushRequest(evt);
  }
  return false;
}

SmsTransReturnCode RpcClient::Start(EgcTrans::CredOptions *opts,
                                               const std::vector<std::string>& ips) const
{
  bool ret = true;
  if (stream_io_)
  {
    //创建grpc channel
    ret = egcChannel_->Connect(opts, ips);
    if (ret)
    {
      //创建stub
      ret = stream_io_->Init();
      if(!ret)
      {
        SLOGE("rpcstream init fail");
        return SmsTransReturnCode::E_RETURN_STUB_NOT_READY;
      }
      //创建线程
      stream_io_->Start();
      return SmsTransReturnCode::E_RETURN_SUCCESS;
    }
  }
  return SmsTransReturnCode::E_RETURN_STREAM_NOT_READY;
}

void RpcClient::SetOnMessage(onMessage msgclk)
{
  if (msgclk)
  {
    on_message_ = std::move(msgclk);
    SLOGI("on_message_ SETTED");
  }
  else
  {
    on_message_ = nullptr;
    SLOGI("on_message_ removed");
  }
}

void RpcClient::SetOnLinkStatus(onLinkStatus linkclk)
{
  if (linkclk)
  {
    on_linkstatus_ = std::move(linkclk);
    SLOGI("on_linkstatus_ added");
  }
  else
  {
    on_linkstatus_ = nullptr;
    SLOGI("on_linkstatus_ removed");
  }
}

} // namespace EgcTrans
