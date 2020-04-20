#pragma once

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

using grpc::Channel;
using grpc::ClientAsyncResponseReader;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::CompletionQueue;
using grpc::Status;

#include "proto/pushGateWay.grpc.pb.h"
#include "proto/pushGateWay.pb.h"

using egc_invoke_func = std::function<void(grpc::CompletionQueue*)>;
using namespace grpc::push::gateway;
using ListenStream = 
grpc::ClientAsyncReaderWriterInterface<PushRegReq, PushData>;

using Credentials = std::unordered_map<std::string, std::string>;


#include "Egc.h"
#include "common/EgcGrpcLog.h"
#include "common/EgcUtils.h"
#include "common/SLog.h"

namespace EgcTrans {

typedef grpc::SslCredentialsOptions CredOptions;

enum ReplyErr {
  REPLY_RES_CLK_OK = 0,
  REPLY_RES_NOT_FIND = 1,
  REPLY_RES_CLK_TIMEOUT = 2,
  REPLY_RES_MAP_EMPTY = 3,
  REPLY_RES_EVT_NULL = 4,
  REPLY_RES_PARSE_FAIL = 5,
  REPLY_RES_NO_CLK = 6,
  REPLY_RES_CLK_ERR = 7,
};

struct RET_REPLY {
  bool ret;
  ReplyErr err;
  StreamURI uri;
  std::string name;
};

enum NetworkStatus {
  NS_UNKOWN = 0,
  NS_DISCONNECT = 1,
  NS_IDLE,
  NS_CHANNEL_CONNECTING,
  NS_STREAM_ESTABLISH_FAIL,
  NS_STREAM_ESTABLISHED,
  NS_PUSHGW_CONNECTING,
  NS_PUSHGW_LOGINED,
  NS_PUSHGW_LOGOUTED = 8,
  NS_PUSHGW_PUSHING,
  NS_PUSHGW_ERR,
  NS_STREAM_ERR,
  NS_STREAM_SHUTDOWN,
  NS_STREAM_RESTART,
};

enum WRITE_ERR {
  WRITE_OK = 0,
  WRITE_NOT_READY = 1,
  WRITE_NO_REQUEST = 2,
  WRITE_UNKOWN_EVT = 3,
  WRITE_STREAM_NO_ESTABLISH = 4,
};

struct RET_WRITE {
  bool ret;
  WRITE_ERR err;
};

} // namespace EgcTrans
