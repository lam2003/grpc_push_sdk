#pragma once
#include "EgcError.h"
#include "EgcRpcDelegate.h"
#include "common/EgcQueue.h"

using Reply = std::shared_ptr<PushData>;


namespace EgcTrans {

/*管理 channel cq 和url*/
class EgcChannel {
public:
  EgcChannel();
  ~EgcChannel();
  bool Connect(EgcTrans::CredOptions *opts,
               const std::vector<std::string>& ips);

public:
  std::shared_ptr<grpc::Channel> channel_;
  std::unique_ptr<grpc::CompletionQueue> cq_;

private:
  std::vector<std::string> urls_;
};

/*从EgcChannel 获取 cq channel ，方便IOTHREAD使用*/
class EgcIO {
public:
  EgcIO();
  ~EgcIO();
  EgcChannel* Channel() const;
  void SetChannel(EgcChannel* channel);
  bool Connect(EgcTrans::CredOptions* opts,
               const std::vector<std::string>& ips);

protected:
  grpc::CompletionQueue* Cqueue();
  std::shared_ptr<grpc::Channel> Transport(); 
  EgcChannel* egc_channel_;
  std::unique_ptr<pushGateway::Stub> stub_;
  std::atomic_bool io_init_{false};
};

class RpcStream : public EgcIO {
public:
  RpcStream(IClientDelegate* listener);
  ~RpcStream();
  bool Init();
  void Start();
  void Stop() const;
  void ShutdownCQ();
  // todo 这个方法存在多线程调用的问题
  bool PushRequest(EgcTrans::IEgcInnerEvt evt);

private:
  std::unique_ptr<std::thread> io_thread_;
  static inline void* GenerateConnectTag(uint32_t connect_sequence_num)
  {
    return reinterpret_cast<void*>((connect_sequence_num << 8) + 0);
  }

  static inline bool IsConnectOperation(void* tag)
  {
    return (((uintptr_t)tag) & 0xFF) == 0;
  }

  static inline uint32_t ConnectSequenceFromConnectTag(void* tag)
  {
    return (uint32_t)(((uintptr_t)tag) >> 8);
  }

  void HandleEvent(bool ok, void* tag);
  void HandleCQTimeout(bool ok);
  void io_loop();

  void RefreshWriteTimestamp();
  bool OpenStream();
  bool ReOpenStream();
  bool CloseStream();
  void setWriteable(std::string caller, bool enable);

  void onLinkStatus(SmsTransLinkStatus status);
  void onPushMessage(RecvPushMessage msg);
  RET_REPLY ParseReply(Reply reply);
  RET_WRITE WriteRequest();
  RET_WRITE WriteNext();
  RET_WRITE checkNextWrite();
  ReplyErr onResponseCallback(std::string ctx, bool& ret,
                              uint32_t resCode = RES_SUCCESS,
                              std::string errDesc = "");

  template <typename ReplyRes>
  bool onResponse(Reply& reply, ReplyRes& res /*, bool& result*/)
  {
    bool ret = res.ParseFromString(reply->msgdata());
    return ret;
  }

  void checkRequestTimeout();
  bool checkUserRequestLogic(int requst_code);

  bool PopRequest(EgcTrans::IEgcInnerEvt& evt);
  void ReadNext();
  void AsyncReadNextMessage() const;
  bool AsyncWriteNextMessage(StreamURI uri, std::string& reqData);
  // std::string getCtx();
  bool PushHeartBeatRequest(std::uint64_t delta);
  bool genLoginRequest(std::string& loginReqData, EgcTrans::IEgcInnerEvt evt);
  bool genJoinGroupRequest(std::string& joingroupReqData,
                           EgcTrans::IEgcInnerEvt evt);
  bool genLeaveGroupRequest(std::string& leavegroupReqData,
                            EgcTrans::IEgcInnerEvt evt);
  bool genPingRequest(std::string& pingData, EgcTrans::IEgcInnerEvt evt);
  void write_writedone_request();
  bool write_ping_request(EgcTrans::IEgcInnerEvt evt);
  bool write_login_request(EgcTrans::IEgcInnerEvt evt);
  bool write_joingroup_request(EgcTrans::IEgcInnerEvt evt);
  bool write_leavegroup_request(EgcTrans::IEgcInnerEvt evt);
  bool write_logout_request(EgcTrans::IEgcInnerEvt evt);
  bool genLogoutRequest(std::string& logoutReqData, EgcTrans::IEgcInnerEvt evt);
  void re_login_();
  std::unique_ptr<grpc::ClientContext> context_;
  std::unique_ptr<ListenStream> stream_;

  uint64_t last_write_timestamp_;
  std::atomic_bool sending_shutdown_ = {false};
  std::atomic_bool exit_loop_{false};
  std::atomic_bool stream_reopen_{false};
  std::atomic_bool stream_init_{false};
  std::atomic_bool stream_established_{false}; //流建立成功了
  std::atomic_bool link_ready_login_ok_{false}; // login成功了，链路已经打通
  std::atomic_bool readable_ = {true};
  std::atomic_bool writeable_ = {true};
  uint64_t suid_;
  egc_uid_t uid_;
  uint32_t connect_sequence_num_;
  ListenStream* prev_io_;

  std::unique_ptr<PushData> response_toRead_;
  NetworkStatus net_status_ = {NS_UNKOWN};
  IClientDelegate* client_delegator_;

  using EgcReqQue =
  moodycamel::ConcurrentQueue<EgcTrans::IEgcInnerEvt, EgcCommon::MyTraits>;
  using EgcRepQue = moodycamel::ConcurrentQueue<Reply>;
  EgcReqQue event_requests_;
  EgcRepQue replys_;

  std::mutex reqmtx_;
  using ReqMap = std::map<std::string, EgcTrans::IEgcInnerEvt>;
  ReqMap reqmap_;
  std::string last_login_data_;
};

} // namespace EgcTrans
