#pragma once
#include <atomic>
#include <iomanip>
#include <iostream>
#include <map>
#include <mutex>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>
#include "Egc.h"
#include "EgcCallback.h"

using clkptr_t = std::shared_ptr<EgcTrans::ResultCallback>;
using ctx2optclkMap_t = std::map<std::string, clkptr_t>;
using sshandler_t = egc_handler_t;
using msghandler_t = egc_handler_t;
using LinkssCb_t = SmsTrans_Linkstatus_Callback;
using MsgCb = SmsTrans_Pushmsg_Callback;

using linkstatus_param_clk_t =
EgcTrans::ParamCallback<SmsTrans_Linkstatus_Callback>;
using msg_param_clk_t =
EgcTrans::ParamCallback<SmsTrans_Pushmsg_Callback>;
using LinkSS_t = std::shared_ptr<linkstatus_param_clk_t>;
using MsgCb_t = std::shared_ptr<msg_param_clk_t>;

struct MsgCbId_t {
public:
  // MsgCbId_t(MsgCbId_t &p)
  // {
  // cb_ptr_ = p.cb_ptr_;
  // id_ = p.id_;
  // }
  MsgCb_t cb_ptr_;
  msghandler_t id_;
  // compare for order.
  bool operator<(const MsgCbId_t& pt) const { return (this->id_ < pt.id_); }
  // bool operator <(const MsgCbId_t pt) const
  // {
  // return (this->id_ < pt.id_);
  // }
};

using MsgCBIDSet = std::set<MsgCbId_t>;
using tupleMp_t = std::map<std::string, MsgCBIDSet>;
using id2keyMap_t = std::map<uint64_t, std::string>;

namespace EgcTrans {
class IListener {
public:
  IListener()
  {
  }

  virtual ~IListener()
  {
  }

  uint64_t GenId()
  {
    static std::atomic_int64_t id = {0};
    return id++;
  }
};

class GroupOptClkListener : public IListener {
public:
  GroupOptClkListener()
  {
  };

  ~GroupOptClkListener()
  {
    //Çå¿Õ
    ctx2optclkMap_t tempJoin;
    ctx2joinMap.swap(tempJoin);
    ctx2joinMap.clear();
    ctx2optclkMap_t tempLeave;
    ctx2leaveMap.swap(tempLeave);
    ctx2leaveMap.clear();
  }

  void AddJoinOpt(std::string ctx, SUC suc, ERR err, void* data);
  clkptr_t PopJoinOpt(std::string ctx);
  void AddLeaveOpt(std::string ctx, SUC suc, ERR err, void* data);
  clkptr_t PopLeaveOpt(std::string ctx);

private:
  ctx2optclkMap_t ctx2joinMap;
  ctx2optclkMap_t ctx2leaveMap;
  std::mutex join_mutex_;
  std::mutex leave_mutex_;
};

class StatusListener : public IListener {
public:
  StatusListener()
  {
  };

  ~StatusListener()
  {
  };
  void AddSSLis(sshandler_t, LinkssCb_t ss, void* arg);
  void Dispatch(SmsTransLinkStatus status);
  bool RemoveSSLis(sshandler_t);

private:
  using linkStatusMap = std::map<sshandler_t, LinkSS_t>;
  linkStatusMap linkssMap_;
  std::mutex status_mutex_;
};

class UidMsgListener : public IListener {
public:
  UidMsgListener()
  {
  }

  ~UidMsgListener()
  {
  }

  void Add(msghandler_t id, std::string servicename, uint64_t uid, MsgCb cb,
           void* arg)
  {
    std::lock_guard<std::mutex> lk(msg_mutex_);
    auto uid_pair = std::make_pair(servicename, uid);
    auto key = getKey(uid_pair);
    id2keyMap_[id] = key;
    auto ptr = std::make_shared<msg_param_clk_t>();
    ptr->set(cb, arg, std::to_string(id) + "_uid_msgcb");
    MsgCbId_t cbid_;
    cbid_.cb_ptr_ = ptr;
    cbid_.id_ = id;

    auto itr = group2cbMp_.find(key);
    if (itr == group2cbMp_.end())
    {
      MsgCBIDSet cbid_set_;
      cbid_set_.insert(cbid_);
      group2cbMp_[key] = cbid_set_;
    }
    else
    {
      auto cbid_set_ = itr->second;
      cbid_set_.insert(cbid_);
    }
  }

  bool Dispatch(std::string servicename, uint64_t uid, const char* data,
                uint32_t size)
  {
    std::lock_guard<std::mutex> lk(msg_mutex_);
    auto uid_pair = std::make_pair(servicename, uid);
    auto key = getKey(uid_pair);
    auto itr = group2cbMp_.find(key);
    if (itr == group2cbMp_.end())
    {
      SLOGE("dispatch fail :not find key {}", key);
      return false;
    }
    auto msgidset = group2cbMp_[key];
    for (auto& entry : msgidset)
    {
      auto ptr = entry.cb_ptr_;
      if (ptr)
      {
        ptr->call(data, size);
      }
    }
    return true;
  }

  bool Remove(msghandler_t id)
  {
    std::lock_guard<std::mutex> lk(msg_mutex_);
    auto itt = id2keyMap_.find(id);
    if (itt == id2keyMap_.end())
    {
      SLOGE("remove fail :not find id {} ", id);
      return false;
    }
    auto key = id2keyMap_[id];
    auto itr = group2cbMp_.find(key);
    if (itr == group2cbMp_.end())
    {
      // not find key
      SLOGE("remove fail :not find key to id {}", id);
      return false;
    }
    auto msgdiset = group2cbMp_[key];

#if 0
	  auto it = std::remove_if(msgdiset.begin(), msgdiset.end(), [id](MsgCbId_t& cbid) {
		if (cbid.id_ == id)
		{
		  return true;
		}
		});
	  msgdiset.erase(it, msgdiset.end());
#endif
    bool ret = false;
    for (auto it = msgdiset.begin(); it != msgdiset.end();)
    {
      if (it->id_ == id)
      {
        it = msgdiset.erase(it);
        ret = true;
      }
      else
      {
        it++;
      }
    }
    return ret;
  }

private:
  typedef std::pair<std::string, uint64_t> UPAIR;
  const char ReservedChar = ',';

  std::string getKey(UPAIR& t)
  {
    std::stringstream ss;
    ss << std::get<0>(t) << ReservedChar << std::get<1>(t);
    return ss.str();
  }

  std::mutex msg_mutex_;
  using id2keyMap_t = std::map<uint64_t, std::string>;
  id2keyMap_t id2keyMap_;
  tupleMp_t group2cbMp_;
};

class GroupMsgListenr : public IListener {
public:
  GroupMsgListenr()
  {
  };

  ~GroupMsgListenr()
  {
  };

  void Add(msghandler_t id, std::string servicename, uint64_t grouptype,
           uint64_t groupid, MsgCb cb, void* arg)
  {
    std::lock_guard<std::mutex> lk(msg_mutex_);
    auto tsii = std::make_tuple(servicename, grouptype, groupid);
    std::string key = getKey(tsii);
    id2keyMap_[id] = key;
    auto ptr = std::make_shared<msg_param_clk_t>();
    ptr->set(cb, arg, std::to_string(id) + "_group_msgcb");
    MsgCbId_t cbid_;
    cbid_.cb_ptr_ = ptr;
    cbid_.id_ = id;
    auto itr = group2cbMp_.find(key);
    if (itr == group2cbMp_.end())
    {
      MsgCBIDSet cbid_set_;
      cbid_set_.insert(cbid_);
      group2cbMp_[key] = cbid_set_;
    }
    else
    {
      auto cbid_set_ = itr->second;
      cbid_set_.insert(cbid_);
    }
  }

  bool Dispatch(std::string servicename, uint64_t grouptype, uint64_t groupid,
                const char* data, uint32_t size)
  {
    {
      std::lock_guard<std::mutex> lk(msg_mutex_);
      auto tsii = std::make_tuple(servicename, grouptype, groupid);
      std::string key = getKey(tsii);
      auto itr = group2cbMp_.find(key);
      if (itr == group2cbMp_.end())
      {
        SLOGE("dispatch fail :not find key {}", key);
        return false;
      }
      auto msgidset = group2cbMp_[key];
      for (auto& entry : msgidset)
      {
        auto ptr = entry.cb_ptr_;
        if (ptr)
        {
          ptr->call(data, size);
        }
      }
    }

    return true;
  }

  bool Remove(msghandler_t id)
  {
    std::lock_guard<std::mutex> lk(msg_mutex_);
    auto itt = id2keyMap_.find(id);
    if (itt == id2keyMap_.end())
    {
      SLOGE("remove fail :not find id {} ", id);
      return false;
    }
    auto key = id2keyMap_[id];
    auto itr = group2cbMp_.find(key);
    if (itr == group2cbMp_.end())
    {
      // not find key
      SLOGE("remove fail :not find key to id {}", id);
      return false;
    }
    auto msgdiset = group2cbMp_[key];

#if 0
	  auto it = std::remove_if(msgdiset.begin(), msgdiset.end(), [id](MsgCbId_t &cbid) {
		if (cbid.id_ == id)
		{
		  return true;
		}
	  });
	  msgdiset.erase(it, msgdiset.end());
#endif
    bool ret = false;
    for (auto it = msgdiset.begin(); it != msgdiset.end();)
    {
      if (it->id_ == id)
      {
        it = msgdiset.erase(it);
        ret = true;
      }
      else
      {
        it++;
      }
    }
    return ret;
  }

private:
  typedef std::tuple<std::string, uint64_t, uint64_t> TSII;
  const char ReservedChar = ',';

  std::string getKey(TSII& t)
  {
    std::stringstream ss;
    ss << std::get<0>(t) << ReservedChar << std::get<1>(t) << ReservedChar
      << std::get<2>(t);
    return ss.str();
  }

  id2keyMap_t id2keyMap_;
  tupleMp_t group2cbMp_;
  std::mutex msg_mutex_;
};
} // namespace EgcTrans
