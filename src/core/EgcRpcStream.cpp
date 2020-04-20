#include "EgcRpcStream.h"


namespace EgcTrans {
EgcChannel::EgcChannel() { cq_.reset(new grpc::CompletionQueue); }

EgcChannel::~EgcChannel()
{
  SLOGW("=====EgcChannel Destroy");
}

bool EgcChannel::Connect(EgcTrans::CredOptions* opts,
                         const std::vector<std::string>& ips)
{
  urls_ = std::move(ips);
  if (channel_.get())
  {
    // channel����Ҫ�ظ�����
    SLOGE("Channel already initialized.");
    return true;
  }
  if (opts != nullptr)
  {
    channel_ = grpc::CreateChannel(urls_[0], grpc::SslCredentials(*opts));
  }
  else
  {
    channel_ = grpc::CreateChannel(urls_[0], grpc::InsecureChannelCredentials());
  }

  if (channel_)
  {
    SLOGI("create grpc channel ok");
    return true;
  }
  SLOGI("create grpc channel fail");
  // channel����ʧ��
  return false;
}

EgcIO::EgcIO() : egc_channel_(nullptr)
{
}

EgcIO::~EgcIO()
{
  if (egc_channel_)
  {
    SLOGW("=====EgcIO Destroy ,but not destroy EgcChannel");
    //��MeshClient�������ͷ�EgcChannel
  }
}

bool EgcIO::Connect(EgcTrans::CredOptions* opts,
                    const std::vector<std::string>& ips)
{
  if (!egc_channel_)
  {
    SLOGE("EgcClient NO EgcChannel Cannot Connect,SetChannel first");
    return false;
  }
  return egc_channel_->Connect(opts, ips);
}

EgcChannel* EgcIO::Channel() const { return egc_channel_; }

std::shared_ptr<grpc::Channel> EgcIO::Transport()
{
  return egc_channel_->channel_;
}

void EgcIO::SetChannel(EgcChannel* channel)
{
  // channel�ı䣬������
  if (egc_channel_ != channel)
  {
    if (egc_channel_)
    {
      delete egc_channel_;
      egc_channel_ = nullptr;
    }
    egc_channel_ = channel;
  }
}

grpc::CompletionQueue* EgcIO::Cqueue() { return egc_channel_->cq_.get(); }

RpcStream::RpcStream(IClientDelegate* listener)
{
  client_delegator_ = listener;
  suid_ = EgcParam::instance()->GetSUid();
  uid_ = EgcParam::instance()->GetUid();
}

RpcStream::~RpcStream()
{
  suid_ = 0;
  uid_ = 0;
  Stop();
  client_delegator_ = nullptr;
}

bool RpcStream::Init()
{
  if (Channel()->channel_)
  {
    stub_ = pushGateway::NewStub(Transport());
    if (stub_)
    {
      io_init_ = true;
      return true;
    }
  }
  else
  {
    SLOGE("EgcChannel null ,cannot create stub");
  }
  return false;
}

void RpcStream::Start()
{
  if (io_init_)
  {
    showChannelState(Transport().get());
    net_status_ = NS_DISCONNECT;
    io_thread_.reset(
      new std::thread(std::bind(&RpcStream::io_loop, this)));
  }
  else
  {
    SLOGE("io_init_ false, no stub ");
  }
}

void RpcStream::ShutdownCQ()
{
#if USE_SEND_SHUTDOWN
  sending_shutdown_ = {true};
  //�ᴥ��cq�˳�
  context_->TryCancel();

  SLOGW("===>Request the shutdown of the queue ");
  Cqueue()->Shutdown();
#endif
}

void RpcStream::Stop() const
{
  SLOGW("RpcStream::Stop,joinThread");
  //cq shutdown ����exit_loop_ Ϊtrue
  //exit_loop_ = true;
  if (io_thread_->joinable())
  {
    //�˳���һ����ر�cq
    // sending_shutdown_ = true;
    if (!exit_loop_)
    {
      SLOGW("RpcStream::Stop exit_loop_ is false ,cq is not shutdown yet ");
    }
    io_thread_->join();
    SLOGW("RpcStream::Stop,joinThread over");
  }
}

void RpcStream::io_loop()
{
  void* tag;
  bool ok;
  gpr_timespec wait = gpr_time_from_millis(50, GPR_TIMESPAN);
  while (!exit_loop_)
  {
    if (!Cqueue())
    {
      SLOGE("CQ NULL");
      return;
    }
    switch (Cqueue()->AsyncNext(&tag, &ok, wait))
    {
    case grpc::CompletionQueue::SHUTDOWN:
      exit_loop_ = true;
      SLOGW(" recv status :cq shutdown ,so exit ioloop thread");
      break;
    case grpc::CompletionQueue::TIMEOUT:
      HandleCQTimeout(ok);
      break;
    case grpc::CompletionQueue::GOT_EVENT:
      // cq����ʱ���ж��Ƿ�Ҫ�ر�cq,���ʱ�������ȷ�ر�cq
      if (!ok)
      {
        if (sending_shutdown_)
        {
          SLOGW("cq err, connection shutdown");
          Cqueue()->Shutdown();
          return;
        }
      }
      HandleEvent(ok, tag);
      break;
    default:
      break;
    }
  }
  SLOGI(" end of io_loop thread ");
  exit_loop_ = true;
}

void RpcStream::onLinkStatus(SmsTransLinkStatus status)
{
  if (client_delegator_)
  {
    if (client_delegator_->on_linkstatus_) client_delegator_->on_linkstatus_(status);
  }
}

void RpcStream::onPushMessage(
  /*const char* data, uint32_t size*/ RecvPushMessage msg)
{
  if (client_delegator_)
  {
    if (client_delegator_->on_message_) client_delegator_->on_message_(msg);
  }
}

void RpcStream::AsyncReadNextMessage() const
{
#if USE_SEND_SHUTDOWN
  if (sending_shutdown_)
  {
    SLOGW("=======already ask shutdown cq, cannot read =====");
    return;
  }
#endif
  stream_->Read(response_toRead_.get(),
                reinterpret_cast<void*>(ETagType::READ));
}

void RpcStream::ReadNext()
{
  if (readable_)
  {
    AsyncReadNextMessage();
    readable_ = false;
  }
  else
  {
    SLOGE("NOT FINISHED ,CANOT READ NEXT");
  }
}

void RpcStream::HandleEvent(bool ok, void* tag)
{
#if 0
	if (!ok)
	{
	  std::cout << "================1===================" << std::endl;

#if USE_SEND_SHUTDOWN
  // if (sending_shutdown_) {
  ////����������⣿������
  ////SLOGI("cq err :connection shutdown");
  //std::cout << "cq err :connection shutdown=2=" << std::endl;
  //Cqueue()->Shutdown();
  //return;
  // }
#endif
	}
#endif
  auto isConnectOpt = (IsConnectOperation(tag));
  uint32_t connSeq;

  ETagType type;
  if (isConnectOpt)
  {
    type = ETagType::CONNECT;
    connSeq = ConnectSequenceFromConnectTag(tag);
  }
  else
  {
    type = static_cast<ETagType>(reinterpret_cast<long>(tag));
  }
  switch (type)
  {
  case ETagType::WRITES_DONE:
    SLOGI("send WRITES_DONE ok");
    WriteNext();
    break;
  case ETagType::CONNECT:
    SLOGI("send CONNECT ok ,cq_ok {}", ok);
    //״̬��д
    setWriteable("ETagType::CONNECT ok", true);
    if (ok)
    {
      //��ȷ��״̬���ٶ�д
      stream_established_ = true;
      if (stream_reopen_)
      {
        if (link_ready_login_ok_)
        {
          SLOGE("stream_reopen_ , link_ready_login_ok_ is ok,relogin");
          //�Ѿ���½���������ǳɹ�������£���Ҫ�ؽ����������������ɹ�����Ҫ���µ�½
          re_login_();
          setWriteable("stream_reopen_ relogin", false);
        }
        else
        {
          SLOGE(
            "send CONNECT ok ,cq_ok {} ,stream_reopen_ is true,but "
            "link_ready_login_ok_ is false,not relogin",
            ok);
        }
        stream_reopen_ = false;
      }
      else
      {
        WriteNext();
      }

      net_status_ = NS_STREAM_ESTABLISHED;
      ReadNext();
      onLinkStatus(SmsTransLinkStatus::E_LINK_CREATE_OK);
    }
    else // cq err,�п����Ƿ������������
    {
      stream_established_ = false;
      net_status_ = NS_STREAM_ESTABLISH_FAIL;
      //����������У�֪ͨ�û�����Щ���󶼷������ˡ� TODO
      // DrainRequestQueue();
      //��ʹfalse��Ҳ��������
      onLinkStatus(SmsTransLinkStatus::E_LINK_CREATE_RETRY);
    }
    break;

  case ETagType::LOGIN:
    {
      WriteNext();
      SLOGI("......LOGIN SEND OK....");
      if (net_status_ == NS_STREAM_ESTABLISHED)
      {
        net_status_ = NS_PUSHGW_CONNECTING;
      }
      //��½��
      onLinkStatus(SmsTransLinkStatus::E_LINK_LOGINTING);
    }
    break;
  case ETagType::LOGOUT:
    {
      WriteNext();
      std::cout << "----LOGOUT OK" << std::endl;
    }
    break;
  case ETagType::JOINGROUP:
    {
      WriteNext();
      std::cout << "......JOINGROUP SEND OK...." << std::endl;
    }
    break;
  case ETagType::LEAVEGROUP:
    {
      WriteNext();
      std::cout << "......LEAVEGROUP SEND OK...." << std::endl;
    }
    break;
  case ETagType::HB:
    {
      WriteNext();
      std::cout << EgcCommon::Now() << "----HB  SEND OK " << std::endl;
    }
    break;
  case ETagType::WRITE:
    {
      WriteNext();
      std::cout << "----WRITE OK" << std::endl;
    }
    break;
  case ETagType::READ:
    {
      SLOGI("--READ---");
      readable_ = true;
      PushData* r = response_toRead_.release();
      if (!ok)
      {
        if (r != nullptr)
        {
          SLOGW("----------READ OK ,BUT ERR---HAS DATA");
          delete r;
        }
        else
        {
          SLOGW("----------READ OK ,BUT ERR---no data--------");
        }
      }
      else
      {
        if (r != nullptr)
        {
          Reply reply;
          reply.reset(r);
          //�ַ���Ϣ
          // PushReply(reply);

          ParseReply(reply);
          RET_WRITE ret;
          ret = WriteRequest();
        }
        else
        {
          SLOGW("CQ OK, BUT READ DATA FAIL ");
        }
        response_toRead_.reset(new PushData());
        ReadNext();
      }
    } // end read
    break;
  default:
    break;
  }
}

void RpcStream::HandleCQTimeout(bool ok)
{
  if (!io_init_)
  {
    // channel stubû����
    SLOGE("HandleCQTimeout not init io ok{}", ok);
    return;
  }
  // if (ok)
  //�����Ƿ�ok��������Ҫ�״δ���
  {
    // streamû����
    if (!stream_init_)
    {
      SLOGI("OpenStream");
      OpenStream();
      stream_init_ = true;
    }
  }
  static uint64_t i = 0;
  if (i == 1000)
  {
    SLOGI(" HandleCQTimeout  ok{}", ok);
    i = 0;
  }
  i++;
  auto rr = checkNextWrite();
  if (!rr.ret)
  {
    static WRITE_ERR wrr = WRITE_OK;
    if (wrr != rr.err)
    {
      SLOGW("checkNextWrite fail {} {} ", rr.err, cqWriteErr(rr.err));
    }
    wrr = rr.err;
  }
  if (exit_loop_)
  {
    //�˳�cq�̣߳���һ������Ҫshutdown cq
    // SLOGE("connection shutdown. seq {}", connect_sequence_num_);
    // Cqueue()->Shutdown();

    //�˳�cqѭ��
    SLOGE("already exit_loop_");
    return;
  }
  else
  {
    //���˳�ѭ����false������ؽ���
    if (!ok)
    {
      std::cout << "HandleCQTimeout cq err " << __LINE__ << std::endl;

      stream_reopen_ = true;
      if (stream_established_)
      {
        SLOGI("===>CQ ERR ReOpenStream");
        ReOpenStream();
      }
      else
      {
        std::cout << "HandleCQTimeout cq err " << __LINE__
          << " but stream not established yet,not reopen" << std::endl;
      }
    }
  }
}

bool RpcStream::OpenStream()
{
  if (!io_init_)
  {
    SLOGE("no stub ,cannot OpenStream");
    return false;
  }

  RefreshWriteTimestamp();
  context_.reset(new ClientContext());
  context_->AddMetadata("suid", std::to_string(suid_));
  connect_sequence_num_++;
  //������
  stream_ = stub_->AsyncPushRegister(context_.get(), Cqueue(),
                                     GenerateConnectTag(connect_sequence_num_));
  if (stream_)
  {
    net_status_ = NS_IDLE; //�������ˣ�����ɶҲû��������IDLE
    return true;
  }
  return false;
}

void RpcStream::RefreshWriteTimestamp()
{
  //ˢ��дʱ���
  last_write_timestamp_ = EgcCommon::getCurHaomiao();
}

bool RpcStream::ReOpenStream()
{
  bool ret1 = CloseStream();
  //�ܾ�����д����
  setWriteable("ReOpenStream", false);
  bool ret2 = OpenStream();
  SLOGW("ReOpenStream: CLOSE {} OPEN {}", ret1, ret2);
  return ret1 && ret2 ? true : false;
}

void RpcStream::setWriteable(std::string caller, bool enable)
{
  if (!enable)
  {
    SLOGW("caller disable write {}", caller);
  }
  writeable_ = enable;
}

bool RpcStream::CloseStream()
{
  if (stream_ != nullptr)
  {
    prev_io_ = stream_.release();
    try
    {
      delete prev_io_;
      prev_io_ = nullptr;
      if (context_ != nullptr)
      {
        auto ptr = context_.release();
        delete ptr;
        ptr = nullptr;
      }
    }
    catch (const std::exception& e)
    {
      SLOGE(" delete prev stream exception  {}", e.what());
      return false;
    }
    return true;
  }
  else
  {
    SLOGE("no stream to close");
  }
  return false;
}

bool RpcStream::PushHeartBeatRequest(std::uint64_t delta)
{
  static uint64_t id = 0;
  IEgcInnerEvt evt = std::make_shared<EgcInternalEvent>();
  evt->event_code = EGC_INTERNAL_EVENT_HEARTBEAT;
  evt->heartbeat.delta = delta;
  evt->heartbeat.uid = uid_;
  evt->context_ = "inner_heartbeat_" + std::to_string(id) + "_" +
    std::to_string(EgcCommon::getCurHaomiao());
  //ע�⣬�����������ã��ı���evt������ʱ�䣬�ǲ��Բ�Ҫ��
#if HB_TEST_AFTER_2_SECONDS
  evt->start_at_ = EgcCommon::getHaomiaoAfterSeconds(2);
#endif
  // TODO û��������ǿ���ؽ�stream��
  evt->setCallback([this](SmsTransCallErrType err, std::string ctx,
                          uint32_t errcode, std::string errdesc)
  {
    SLOGW(" HEARTBEAT  {} errcode {} errdesc{}", callerr2str(err), errcode,
          errdesc);
  });
  return PushRequest(evt);
}

RET_WRITE RpcStream::WriteRequest()
{
  //�ж��Ƿ�Ҫ������
  RET_WRITE rr;
  rr.ret = true;
  rr.err = WRITE_ERR::WRITE_OK;
  auto delta = EgcCommon::getCurHaomiao() - last_write_timestamp_;
#if DISABLE_SEND_HB
  if (delta > PUSH_SVR_HEARTBEAT_HAOMIAO) {
    // std::cout << "=== delta " << delta << " BUT not call
    // NotifyHeartBeatEvent" << std::endl;
  }
#else
  if (delta > PUSH_SVR_HEARTBEAT_HAOMIAO)
  {
    std::cout << "=== delta " << delta << " call NotifyHeartBeatEvent"
      << std::endl;
    PushHeartBeatRequest(delta);
  }
#endif
  if (!writeable_)
  {
    SLOGW("WRITE NOT READY");
    rr.err = WRITE_ERR::WRITE_NOT_READY;
    rr.ret = false;
    return rr;
  }
  EgcTrans::IEgcInnerEvt evt;
  bool ret = PopRequest(evt);
  if (!ret)
  {
    rr.err = WRITE_ERR::WRITE_NO_REQUEST;
    // SLOGW("pop fail ,no request");
    rr.ret = false;
    return rr;
  }

  //�Ƿ���Ҫ��С�����������̰߳�ȫ���Ƚ��õ���д��Ȼ���cq��д����
  setWriteable("WriteRequest from evt", false);
  std::string ctx = evt->getCtx();
  switch (evt->event_code)
  {
  case EgcTrans::EGC_INTERNAL_EVENT_HEARTBEAT:
#if DISABLE_SEND_HB
      setWriteable("WriteRequest from evt", true);
      goto done;
#else
    write_ping_request(evt);
#endif
    break;
  case EgcTrans::EGC_INTERNAL_EVENT_LOGIN:
    write_login_request(evt);
    break;
  case EgcTrans::EGC_INTERNAL_EVENT_JOIN:
    write_joingroup_request(evt);
    break;
  case EgcTrans::EGC_INTERNAL_EVENT_LEAVE:
    write_leavegroup_request(evt);
    break;
  case EgcTrans::EGC_INTERNAL_EVENT_LOGOUT:
    write_logout_request(evt);
    break;
  default:
    SLOGE("unkown evt ,not send request");
    // EVENT��SDK�ڲ���װ��Ӧ�ò�����δ֪���
    rr.err = WRITE_ERR::WRITE_UNKOWN_EVT;
    rr.ret = false;
    //�������Ҫ���������˿ɼ���д
    setWriteable("WriteRequest from evt", true);
    goto done;
    // break;
  }
  //������map
  {
    std::lock_guard<std::mutex> lk(reqmtx_);
    reqmap_[ctx] = evt;
  }
done:
  //����ʱ���
  checkRequestTimeout();
  return rr;
}

bool RpcStream::genLoginRequest(std::string& loginReqData,
                                EgcTrans::IEgcInnerEvt evt)
{
  LoginRequest req;
  req.set_uid(evt->login.uid_);
  req.set_appid(std::to_string(evt->login.appid_));
  req.set_suid(suid_);
  req.set_appkey(EgcTrans::EgcParam::instance()->GetAppKey());
  req.set_termnialtype(getProtoTT());
  req.set_account(std::string(evt->login.account_));
  std::string pw = std::string(evt->login.passwd_);
  req.set_password(pw);
  req.set_cookie(std::string(evt->login.token_));
  req.set_context(evt->context_);
  // SerializeToString
  return req.SerializeToString(&loginReqData);
}

bool RpcStream::write_login_request(EgcTrans::IEgcInnerEvt evt)
{
  SLOGW("---write_login_request-----");
  bool ret = genLoginRequest(last_login_data_, evt);
  if (ret)
    AsyncWriteNextMessage(StreamURI::PPushGateWayLoginURI, last_login_data_);
  return ret;
}

void RpcStream::re_login_()
{
  SLOGW("---re_login_request-----");
  static uint64_t id = 80000;
  IEgcInnerEvt evt = std::make_shared<EgcInternalEvent>();
  evt->event_code = EGC_INTERNAL_EVENT_LOGIN;
  evt->context_ = "re_login_uid_" +
    std::to_string(EgcTrans::EgcParam::instance()->GetUid()) +
    "_" + std::to_string(id) + "_" +
    std::to_string(EgcCommon::getCurHaomiao());
  LoginRequest req;
  req.set_uid(EgcTrans::EgcParam::instance()->GetUid());
  req.set_appid(std::to_string(EgcTrans::EgcParam::instance()->GetAppid()));
  req.set_suid(suid_);
  req.set_appkey(EgcTrans::EgcParam::instance()->GetAppKey());
  req.set_termnialtype(getProtoTT());
  req.set_account(EgcTrans::EgcParam::instance()->GetUserAccout());
  std::string pw = std::string(EgcTrans::EgcParam::instance()->GetUserPasswd());
  req.set_password(pw);
  req.set_cookie(std::string(EgcTrans::EgcParam::instance()->GetToken()));
  std::string reqstr;
  req.SerializeToString(&reqstr);
  AsyncWriteNextMessage(StreamURI::PPushGateWayLoginURI, reqstr);
}

bool RpcStream::write_ping_request(EgcTrans::IEgcInnerEvt evt)
{
  SLOGW("---write_ping_request-----");
  std::string ping;
  bool ret = genPingRequest(ping, evt);
  if (ret) AsyncWriteNextMessage(StreamURI::PPushGateWayPingURI, ping);
  return ret;
}

bool RpcStream::genLogoutRequest(std::string& logoutReqData,
                                 EgcTrans::IEgcInnerEvt evt)
{
  LogoutRequest req;
  req.set_uid(evt->login.uid_);
  req.set_appid(std::to_string(evt->login.appid_));
  req.set_suid(suid_);
  req.set_appkey(EgcTrans::EgcParam::instance()->GetAppKey());
  req.set_termnialtype(getProtoTT());
  std::string pw = std::string(evt->login.passwd_);
  req.set_context(evt->context_);
  return req.SerializeToString(&logoutReqData);
}

bool RpcStream::write_logout_request(EgcTrans::IEgcInnerEvt evt)
{
  SLOGW("---write_logout_request-----");
  std::string logout;
  bool ret = genLogoutRequest(logout, evt);
  if (ret) AsyncWriteNextMessage(StreamURI::PPushGateWayLogoutURI, logout);
  return ret;
}

bool RpcStream::genPingRequest(std::string& pingData,
                               EgcTrans::IEgcInnerEvt evt)
{
  PingRequest ping;
  ping.set_suid(suid_);
  ping.set_context(evt->context_);
  ping.set_uid(evt->heartbeat.uid);
  return ping.SerializeToString(&pingData);
}

bool RpcStream::genJoinGroupRequest(std::string& joingroupReqData,
                                    EgcTrans::IEgcInnerEvt evt)
{
  JoinGroupRequest req;
  req.set_suid(suid_);
  req.set_context(evt->context_);
  UserGroup* usergroup = req.add_usergroupset();
  usergroup->set_usergroupid(evt->join.userGroupId);
  usergroup->set_usergrouptype(evt->join.userGroupType);
  return req.SerializeToString(&joingroupReqData);
}

bool RpcStream::write_joingroup_request(EgcTrans::IEgcInnerEvt evt)
{
  SLOGI("write joingroup request");
  std::string join;
  bool ret = genJoinGroupRequest(join, evt);
  if (ret) AsyncWriteNextMessage(StreamURI::PPushGateWayJoinGroupURI, join);
  return ret;
}

bool RpcStream::genLeaveGroupRequest(std::string& leavegroupReqData,
                                     EgcTrans::IEgcInnerEvt evt)
{
  LeaveGroupRequest req;
  req.set_suid(suid_);
  req.set_context(evt->context_);
  UserGroup* usergroup = req.add_usergroupset();
  usergroup->set_usergroupid(evt->join.userGroupId);
  usergroup->set_usergrouptype(evt->join.userGroupType);
  return req.SerializeToString(&leavegroupReqData);
}

bool RpcStream::write_leavegroup_request(EgcTrans::IEgcInnerEvt evt)
{
  SLOGI("write leaveroup request");
  std::string leave;
  bool ret = genLeaveGroupRequest(leave, evt);
  if (ret) AsyncWriteNextMessage(StreamURI::PPushGateWayLeaveGroupURI, leave);
  return ret;
}

void RpcStream::write_writedone_request()
{
  // if (alive_)
  {
    stream_->WritesDone(reinterpret_cast<void*>(ETagType::WRITES_DONE));
  }
  // else {
  //  SLOGE("CQ NOT ALIVE,CANNOT SEND WRITEDONE ");
  //}
}

bool RpcStream::AsyncWriteNextMessage(StreamURI uri, std::string& reqData)
{
#if USE_SEND_SHUTDOWN
  if (sending_shutdown_)
  {
    SLOGW("===already==ASK SHUTDOWN  ,canot not write ====");
    return true;
  }
#endif
  //�ⲿ��writedone��Ȼ����shutdown��
  // if (cmd == "quit") {
  //  push_bi_stream_->WritesDone(reinterpret_cast<void*>(Type::WRITES_DONE));
  //  return false;
  //}

  // Data we are sending to the server.
  PushRegReq req;
  req.set_uri(uri);
  req.set_msgdata(reqData);
  CQTag tag = Uri2Type(uri);
  ETagType type = tag.type_;
  SLOGI("AsyncWriteNextMessage uri {} type {} ", tag.desc, (int)type);
  stream_->Write(req, reinterpret_cast<void*>(type));
  return true;
}

RET_WRITE RpcStream::checkNextWrite()
{
  if (stream_established_)
  {
    RET_WRITE rr = WriteRequest();
    return rr;
  }
  //��û����ʱ������Ҫ������
  RET_WRITE rr = {false, WRITE_STREAM_NO_ESTABLISH};
  return rr;
}

RET_WRITE RpcStream::WriteNext()
{
  RefreshWriteTimestamp();
  setWriteable("WriteNext", true);
  if (stream_established_)
  {
    RET_WRITE rr = WriteRequest();
    return rr;
  }
  //��û����ʱ������Ҫ������
  RET_WRITE rr = {false, WRITE_STREAM_NO_ESTABLISH};
  return rr;
}

RET_REPLY RpcStream::ParseReply(Reply reply)
{
  RET_REPLY rr;
  // std::atomic_bool  parse_res_err_ = false;
  SmsTransPushMsgType msgtype;
  StreamURI uri = reply->uri();
  rr.uri = uri;
  rr.ret = true;
  rr.err = REPLY_RES_CLK_OK;
  std::string name;
  std::atomic_bool is_push_msg_{false};
  switch (uri)
  {
  case PPushGateWayLoginResURI:
    {
      rr.name = "LOGIN_RES";
      LoginResponse res;
      auto ret = onResponse(reply, res);
      //����Ӧ��ɹ�
      if (ret)
      {
        std::string ctx = res.context();
        //�����֪ͨ����
        if (res.rescode() != RES_SUCCESS)
        {
          rr.err = onResponseCallback(
            ctx, rr.ret, res.rescode(),
            std::string("loginres err:") + std::to_string(res.rescode()));
        }
        else //һ������
        {
          rr.err = onResponseCallback(ctx, rr.ret);
        }
        if (rr.ret)
        {
          link_ready_login_ok_ = true;
          SLOGI("LOGIN RES onResponseCallback OK");
          if (net_status_ == NS_PUSHGW_CONNECTING ||
            net_status_ == NS_PUSHGW_LOGOUTED)
          {
            // LOGIN Res OK,��½�ɹ���
            net_status_ = NS_PUSHGW_LOGINED;
          }
          onLinkStatus(SmsTransLinkStatus::E_LINK_LOGIN_OK);
        }
        else
        {
          SLOGE("LOGIN RES onResponseCallback FAIL");
          onLinkStatus(SmsTransLinkStatus::E_LINK_LOGIN_FAIL);
        }
      }
      else
      {
        //���������ɳ�ʱ������
        // SLOGE("onResponse ERR {}", rr.err);
        rr.err = REPLY_RES_PARSE_FAIL;
      }
    }
    break;
  case PPushGateWayLogoutResURI:
    {
      rr.name = "LOGOUT_RES";
      LogoutResponse res;
      auto ret = onResponse(reply, res);
      if (ret)
      {
        std::string ctx = res.context();
        if (res.rescode() != RES_SUCCESS)
        {
          //�д���Ӧ����
          rr.err = onResponseCallback(
            ctx, rr.ret, res.rescode(),
            "logoutres err:" + std::to_string(res.rescode()));
        }
        else
        {
          //����Ӧ��
          rr.err = onResponseCallback(ctx, rr.ret);
        }
        //�жϻص����
        if (rr.ret)
        {
          link_ready_login_ok_ = false;
          onLinkStatus(SmsTransLinkStatus::E_LINK_LOGOUT_OK);
          //�û��ǳ��ɹ���
          net_status_ = NetworkStatus::NS_PUSHGW_LOGOUTED;
        }
        else
        {
          //�ǳ�������
          onLinkStatus(SmsTransLinkStatus::E_LINK_LOGOUT_FAIL);
        }
      }
      else
      {
        //����������
        rr.err = REPLY_RES_PARSE_FAIL;
      }
    }
    break;
  case PPushGateWayJoinGroupResURI:
    {
      rr.name = "JOIN_RES";
      JoinGroupResponse res;
      rr.ret = onResponse(reply, res);
      if (rr.ret)
      {
        std::string ctx = res.context();
        rr.err = onResponseCallback(ctx, rr.ret);
      }
      else
      {
        rr.err = REPLY_RES_PARSE_FAIL;
      }
    }
    break;
  case PPushGateWayPongURI:
    {
      rr.name = "PING_RES";
      PongResponse res;
      rr.ret = onResponse(reply, res);
      if (rr.ret)
      {
        std::string ctx = res.context();
        rr.err = onResponseCallback(ctx, rr.ret);
      }
      else
      {
        rr.err = REPLY_RES_PARSE_FAIL;
      }
    }
    break;

  case PPushGateWayLeaveGroupResURI:
    {
      rr.name = "LEAVEGROUP_RES";
      LeaveGroupResponse res;
      rr.ret = onResponse(reply, res);
      if (rr.ret)
      {
        std::string ctx = res.context();
        rr.err = onResponseCallback(ctx, rr.ret);
      }
      else
      {
        rr.err = REPLY_RES_PARSE_FAIL;
      }
    }
    break;
  case PPushGateWayPushDataByGroupURI:
    rr.name = "PushDataByGroup";
    msgtype = SmsTransPushMsgType::PUSH_MSG_BY_GROUP;
    is_push_msg_ = true;
    break;
  case PPushGateWayPushDataByUidURI:
    rr.name = "PushDataByUID";
    msgtype = SmsTransPushMsgType::PUSH_MSG_BY_UID;
    is_push_msg_ = true;
    break;
  default:
    rr.name = "unkown RES :" + std::to_string(uri);
    break;
  }
  if (is_push_msg_)
  {
    {
      RecvPushMessage msg;
      msg.msgtype = msgtype;
      msg.uid = reply->uid();
      msg.suid = reply->suid();
      //����˴��ڲ���uid��ֻ��suid�����
      if (msg.uid == 0)
      {
        if (msg.suid == suid_)
        {
          msg.uid = EgcTrans::EgcParam::instance()->GetUid();
        }
        else
        {
          SLOGE("no uid and mismatch suid in pushdata {} but our suid  {}",
                msg.suid, suid_);
        }
      }

      msg.groupId = reply->groupid();
      msg.groupType = reply->grouptype();
      msg.seqNum = reply->seqnum();
      msg.serverId = reply->serverid();
      msg.serviceName = reply->servicename();
      msg.msgData = reply->msgdata();
      if (client_delegator_)
      {
        if (reply)
        {
          client_delegator_->on_message_(msg);
          rr.ret = true;
        }
        else
        {
          rr.ret = false;
          SLOGE("NO DATA  {}", rr.name);
        }
      }
      else
      {
        SLOGE("NO on_message_ CALLBACK ");
      }
    }
  }
  if (rr.err == REPLY_RES_PARSE_FAIL)
  {
    SLOGI("parseReply: {} parse err rr.name {} rr.ret {}", Uri2Desc(uri),
          rr.name, rr.ret);
  }
  else
  {
    SLOGI("parseReply: {} ok, rr.name {} rr.ret {}", Uri2Desc(uri), rr.name,
          rr.ret);
  }
  return rr;
}

void RpcStream::checkRequestTimeout()
{
  std::lock_guard<std::mutex> lk(reqmtx_);
  if (!reqmap_.empty())
  {
    auto it = reqmap_.begin();
    while (it != reqmap_.end())
    {
      auto entry = *it;
      auto delta = EgcCommon::getCurHaomiao() - entry.second->start_at_;
      if (delta > RPC_CALL_CHECK_TIMEOUT)
      {
        auto ctx = entry.first;
        auto code = entry.second->event_code;
        auto clk = entry.second->evtclk_;
        if (clk)
        {
          clk(SmsTransCallErrType::CALL_TIMEOUT, ctx, RES_TIMEOUT,
              "checkRequestTimeout set timeout");
          SLOGW("event {} ctx {} timeout callback",
                EgcTrans::evt_code2str(code), ctx);
        }
        else
        {
          SLOGW("event {} ctx {} timeout not find callback",
                EgcTrans::evt_code2str(code), ctx);
        }
        //����
        it = reqmap_.erase(it);
      }
      else
      {
        ++it;
      }
    }
  }
}

bool RpcStream::checkUserRequestLogic(int requst_code)
{
  //�ڲ����󣬷��С�
  if (requst_code == EgcTrans::EGC_INTERNAL_EVENT_HEARTBEAT)
  {
    return true;
  }
  //ֻ����LOGIN����ͨ��
  if (net_status_ == NS_STREAM_ESTABLISHED)
  {
    if (requst_code == EgcTrans::EGC_INTERNAL_EVENT_LOGIN)
    {
      return true;
    }
    return false;
  }
  //�û��Ѿ�����JOIN�ҵ���LOGIN
  //���ͳɹ������ʱ����Ҫ��LOGINRES���������ܷ�������
  if (net_status_ == NS_PUSHGW_CONNECTING)
  {
    //���ǿ����ظ���LOGIN����ģ������Ķ�����
    if (requst_code == EgcTrans::EGC_INTERNAL_EVENT_LOGIN)
    {
      return true;
    }
    return false;
  }
  //�Ѿ����͵�¼���յ���Ӧ���ˡ�
  if (net_status_ == NS_PUSHGW_LOGINED)
  {
    return true;
  }
  if (net_status_ == NS_PUSHGW_LOGOUTED)
  {
    //�Ѿ����͵ǳ��������յ���Ӧ�𣬿�������login����
    if (requst_code == EgcTrans::EGC_INTERNAL_EVENT_LOGIN)
    {
      return true;
    }
  }
  SLOGE("unkown logic ,cur netstat {}", net_status_);
  return false;
}

//�յ�Ӧ��ʱ��������USER�����Ӧ����Ϣ����鲢�ص�
ReplyErr RpcStream::onResponseCallback(std::string ctx, bool& ret,
                                       uint32_t resCode, std::string errDesc)
{
  std::lock_guard<std::mutex> lk(reqmtx_);
  ret = true;
  ReplyErr err;
  if (!reqmap_.empty())
  {
    auto now = EgcCommon::getCurHaomiao();
    auto itr = reqmap_.find(ctx);
    if (itr == reqmap_.end())
    {
      SLOGE(
        "{} not in reqmap !!! "
        "�Ѿ���Ϊ��ʱ��������ˣ����߷�����쳣��",
        ctx);
      ret = false;
      err = REPLY_RES_NOT_FIND;
    }
    else
    {
      //�ҵ���
      EgcTrans::IEgcInnerEvt evt = (*itr).second;
      if (evt.get())
      {
        // �ڲ����ݣ��������Ӧ�ò�����
        if (!evt->evtclk_)
        {
          ret = false;
          err = REPLY_RES_NO_CLK;
          return err;
        }
        auto delta = now - evt->start_at_;
        SLOGW("CTX {} res callback delta {}", ctx, delta);
        if (delta > kConnectionTimeoutInHaomiao)
        {
          //��ʱ��
          evt->evtclk_(SmsTransCallErrType::CALL_TIMEOUT, ctx, RES_TIMEOUT,
                       "onResponseCallback think timeout");
          err = REPLY_RES_CLK_TIMEOUT;
        }
        else
        {
          //���óɹ�
          if (resCode == RES_SUCCESS)
          {
            evt->evtclk_(SmsTransCallErrType::CALL_OK, ctx, RES_SUCCESS,
                         "RES_SUCCESS");
            err = REPLY_RES_CLK_OK;
            ret = true;
          }
          else
          {
            evt->evtclk_(SmsTransCallErrType::CALL_ERR, ctx, resCode, errDesc);
            err = REPLY_RES_CLK_ERR;
            ret = false;
          }
        }
      }
      else
      {
        SLOGE("ERROR !!!JOIN EVT IS NULL !!!");
        //�������Ӧ�ò�����
        ret = false;
        err = REPLY_RES_EVT_NULL;
      }
      //����
      reqmap_.erase(itr);
    }
  }
  else
  {
    SLOGE("REQ MAP IS EMPTY !!!!�����ˣ������Ƿ��������쳣��");
    //�����Ӧ���������ˣ������Ƿ��������쳣����
    err = REPLY_RES_MAP_EMPTY;
  }
  return err;
}

bool RpcStream::PushRequest(EgcTrans::IEgcInnerEvt evt)
{
  if (!checkUserRequestLogic(evt->event_code))
  {
    SLOGE("REQUEST LOGIC CHECK FAIL");
    return false;
  }

  if (stream_established_)
  {
    SLOGI("pushRequest enqueue {}", EgcTrans::evt_code2str(evt->event_code));
    event_requests_.enqueue(evt);
  }
  else
  {
    SLOGE("illegal: should create stream rpc firstly");
    return false;
  }
  return true;
}

bool RpcStream::PopRequest(EgcTrans::IEgcInnerEvt& evt)
{
  return event_requests_.try_dequeue(evt);
}
} // namespace EgcTrans
