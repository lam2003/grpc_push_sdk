#pragma once
#include "EgcRpcInc.h"
#include "common/PlatformDefine.h"
namespace EgcTrans {
enum class ETagType {
  UNKOWN = 0,
  READ = 1,
  WRITE = 2,
  CONNECT = 3,
  WRITES_DONE = 4,
  FINISH = 5,
  HB = 6,
  LOGIN = 7,
  JOINGROUP = 8,
  LEAVEGROUP = 9,
  LOGOUT = 10,
};

struct CQTag {
  ETagType type_;
  std::string desc;
};
UserTerminalType getProtoTT();
std::string Uri2Desc(StreamURI uri);
CQTag Uri2Type(StreamURI uri);
uint64_t genSuid(uint32_t uid, uint16_t terminaltype);
void showChannelState(grpc::Channel* channel);
std::string cqWriteErr(WRITE_ERR err);
}  // namespace EgcTrans
