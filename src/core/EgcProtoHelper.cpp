#include <chrono>
#include <string>
#include "core/EgcProtoHelper.h"

namespace EgcTrans {
#define AS (std::to_string)
	
UserTerminalType getProtoTT() {
  UserTerminalType tt;
  EgcCommon::TerminalType type = EgcCommon::getTerminalType();
  switch (type) {
    case EgcCommon::E_TT_UNKNOWN:
      tt = UTT_UNKNOWN;
      break;
    case EgcCommon::E_TT_WINDOWS_32:
      tt = UTT_DESKTOP;
      break;
    case EgcCommon::E_TT_WINDOWS_64:
      tt = UTT_DESKTOP;
      break;
    // case EgcCommon::E_TT_IOS:
    //   tt = UTT_MOBILEPHONE;
    //   break;
    // case EgcCommon::E_TT_ANDROID:
    //   tt = UTT_MOBILEPHONE;
    //   break;
    // case EgcCommon::E_TT_IOS_SIMULATOR:
    //   tt = UTT_MOBILEPHONE;
    //   break;
    case EgcCommon::E_TT_LINUX:
      tt = UTT_SERVER;
      break;
    case EgcCommon::E_TT_UNIX:
      tt = UTT_SERVER;
      break;
    case EgcCommon::E_TT_MAC:
      tt = UTT_SERVER;
      break;
    case EgcCommon::E_TT_APPLE_UNKNOWN:
      tt = UTT_UNKNOWN;
      break;
    case EgcCommon::E_TT_POSIX_UNKNOWN:
      tt = UTT_UNKNOWN;
      break;
    default:
      tt = UTT_UNKNOWN;
      break;
  }
  return tt;
}

std::string Uri2Desc(StreamURI uri) {
  std::string desc;

  switch (uri) {
    case StreamURI::PPushGateWayLoginURI:
      desc = "LOGIN_REQUEST";
      break;

    case StreamURI::PPushGateWayJoinGroupURI:
      desc = "JOINGROUP_REQUEST";
      break;
    case StreamURI::PPushGateWayPingURI:
      desc = "HEARTBEAT_REQUEST";
      break;
    case StreamURI::PPushGateWayLogoutURI:
      desc = "LOGOUT_REQUEST";
      break;
    case StreamURI::PPushGateWayLeaveGroupURI:
      desc = "LEAVEGROUP_REQUEST";
      break;
    case StreamURI::PPushGateWayLoginResURI:
      desc = "LOGIN_RESPONSE";
      break;
    case StreamURI::PPushGateWayJoinGroupResURI:
      desc = "JOINGROUP_RESPONSE";
      break;
    case StreamURI::PPushGateWayLeaveGroupResURI:
      desc = "LEAVEGROUP_RESPONSE";
      break;
    case StreamURI::PPushGateWayLogoutResURI:
      desc = "LOGOUT_RESPONSE";
      break;
    case StreamURI::PPushGateWayPongURI:
      desc = "PONG";
      break;
    case PPushGateWayPushDataByGroupURI:
      desc = "PUSH_DATA_GROUP";
      break;
    case PPushGateWayPushDataByUidURI:
      desc = "PUSH_DATA_UID";
      break;
    default:
      desc = "unkown uri:" + std::to_string((int)uri);
      break;
  }
  return std::move(desc);
}
CQTag Uri2Type(StreamURI uri) {
  std::string desc;
  ETagType type;

  switch (uri) {
    case StreamURI::PPushGateWayLoginURI:
      type = ETagType::LOGIN;
      desc = "LOGIN_REQUEST";
      break;

    case StreamURI::PPushGateWayJoinGroupURI:
      type = ETagType::JOINGROUP;
      desc = "JOINGROUP_REQUEST";
      break;
    case StreamURI::PPushGateWayPingURI:
      type = ETagType::HB;
      desc = "HEARTBEAT_REQUEST";
      break;
    case StreamURI::PPushGateWayLeaveGroupURI:
      type = ETagType::LEAVEGROUP;
      desc = "LEAVEGROUP_REQUEST";
      break;
    case StreamURI::PPushGateWayLogoutURI:
      type = ETagType::LOGOUT;
      desc = "LOGOUT_REQUEST";
      break;
    default:
      //��Ȼ��δ֪������ʹ��WRITE���
      type = ETagType::WRITE;
      desc = "unkown uri:" + std::to_string((int)uri);
      break;
  }
  CQTag tag = {type, desc};
  return std::move(tag);
}
std::string PushData2String(PushData data) {
  std::string str = "uri: {" + std::to_string(data.uri()) + "} " +
                    Uri2Desc(data.uri()) + "] msg(" + data.msgdata() + ") " +
                    std::to_string(data.suid()) + " " + AS(data.uid()) +
                    AS(data.grouptype()) + " " + AS(data.groupid()) + " " +
                    data.servicename() + " " + (data.msgdata()) + " " +
                    AS(data.seqnum()) + " " + (data.serverid());
  std::cout << "pushresdata:" << str << std::endl;
  return str;
}

gpr_timespec GRPCTime(uint32_t duration_msec) {
  gpr_timespec ts;
  grpc::Timepoint2Timespec(std::chrono::system_clock::now() +
                               std::chrono::milliseconds(duration_msec),
                           &ts);
  return ts;
}
void showChannelState(grpc::Channel* channel) {
  if (!channel) return;
  auto state = channel->GetState(true);
  SLOGW("show channel state : {}", state);
}
std::string cqWriteErr(WRITE_ERR err) {
  std::string name;
  switch (err) {
    case EgcTrans::WRITE_OK:
      name = "WRITE_OK";
      break;
    case EgcTrans::WRITE_NOT_READY:
      name = "WRITE_NOT_READY";
      break;
    case EgcTrans::WRITE_NO_REQUEST:
      name = "WRITE_NO_REQUEST";
      break;
    case EgcTrans::WRITE_UNKOWN_EVT:
      name = "WRITE_UNKOWN_EVT";
      break;
    case EgcTrans::WRITE_STREAM_NO_ESTABLISH:
      name = "WRITE_STREAM_NO_ESTABLISH";
      break;
    default:
      name = "incrediable WRITE_ERR";
      break;
  }
  return name;
}
std::string get_metadata_variable(
    const std::multimap<grpc::string_ref, grpc::string_ref>& metadata,
    const std::string& key) {
  auto i = metadata.find(key);
  if (i == metadata.end()) return {};

  return std::string(i->second.data());
}

uint64_t genSuid(uint32_t uid, uint16_t terminaltype) {
  uint64_t suid = 0;
  uint64_t type = uint64_t(terminaltype);
  suid |= type << (32 + 8);
  suid |= uint64_t(uid) & 0x00000000FFFFFFFF;
  return suid;
}
}  // namespace EgcTrans