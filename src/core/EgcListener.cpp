#include "EgcListener.h"

namespace EgcTrans {

void GroupOptClkListener::AddJoinOpt(std::string ctx, SUC suc, ERR err,
                                     void* data)
{
  std::lock_guard<std::mutex> lk(join_mutex_);
  auto ptr = std::make_shared<EgcTrans::ResultCallback>();
  ptr->set(suc, err, data, ctx);
  ctx2joinMap[ctx] = ptr;
}

clkptr_t GroupOptClkListener::PopJoinOpt(std::string ctx)
{
  std::lock_guard<std::mutex> lk(join_mutex_);
  auto itr = ctx2joinMap.find(ctx);
  if (itr == ctx2joinMap.end())
  {
    return std::shared_ptr<EgcTrans::ResultCallback>();
  }
  auto ptr = itr->second; // make copy of shared_ptr
  ctx2joinMap.erase(itr); // erase
  return ptr;
}

void GroupOptClkListener::AddLeaveOpt(std::string ctx, SUC suc, ERR err,
                                      void* data)
{
  std::lock_guard<std::mutex> lk(join_mutex_);
  auto ptr = std::make_shared<EgcTrans::ResultCallback>();
  ptr->set(suc, err, data, ctx);
  ctx2leaveMap[ctx] = ptr;
}

clkptr_t GroupOptClkListener::PopLeaveOpt(std::string ctx)
{
  std::lock_guard<std::mutex> lk(join_mutex_);
  auto itr = ctx2leaveMap.find(ctx);
  if (itr == ctx2leaveMap.end())
  {
    return std::make_shared<EgcTrans::ResultCallback>();
  }
  auto ptr = itr->second; // make copy of shared_ptr
  ctx2leaveMap.erase(itr); // erase
  return ptr;
}

void StatusListener::AddSSLis(sshandler_t id, LinkssCb_t ss, void* arg)
{
  std::lock_guard<std::mutex> lk(status_mutex_);
  auto ptr = std::make_shared<linkstatus_param_clk_t>();
  ptr->set(ss, arg, std::to_string(id) + "statuslink_cb");
  linkssMap_[id] = ptr;
}

bool StatusListener::RemoveSSLis(sshandler_t id)
{
  std::lock_guard<std::mutex> lk(status_mutex_);
  auto itr = linkssMap_.find(id);
  if (itr == linkssMap_.end())
  {
    return false;
  }
  linkssMap_.erase(itr); // erase
  return true;
}

void StatusListener::Dispatch(SmsTransLinkStatus status)
{
  std::lock_guard<std::mutex> lk(status_mutex_);
  for (auto& entry : linkssMap_)
  {
    entry.second->call(status);
  }
}
} // namespace EgcTrans
