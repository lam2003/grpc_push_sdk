#include "EgcInternalEvent.h"
#include "EgcRpcClient.h"
#include "common/EgcUtils.h"
#include "common/SLog.h"

namespace EgcTrans {
std::string callerr2str(SmsTransCallErrType err)
{
  std::string name;
  switch (err)
  {
  case SmsTransCallErrType::CALL_OK:
    name = " CALL SUCCESS";
    break;
  case SmsTransCallErrType::CALL_ILLEGAL:
    name = " CALL ILLEGAL ";
    break;
  case SmsTransCallErrType::CALL_TIMEOUT:
    name = " CALL TIMEOUT ";
    break;
  default:
    name = " UNKOWN ,SHOULD NOT HAPPEN ";
    break;
  }
  return std::move(name);
}

std::string evt_code2str(int code)
{
  std::string evtname;
  switch (code)
  {
  case EGC_INTERNAL_EVENT_HEARTBEAT:
    evtname = "EVT_HEARTBEAT";
    break;
  case EGC_INTERNAL_EVENT_JOIN:
    evtname = "EVT_USER_JOINGROUP";
    break;
  case EGC_INTERNAL_EVENT_LOGIN:
    evtname = "EVT_USER_LOGIN";
    break;
  case EGC_INTERNAL_EVENT_LOGOUT:
    evtname = "EVT_USER_LOGOUT";
    break;
  case EGC_INTERNAL_EVENT_LEAVE:
    evtname = "EVT_USER_LEAVEGROUP";
    break;
  default:
    evtname = "EVT_UNKOWN";
    break;
  }
  return std::move(evtname);
}
} // namespace EgcTrans
