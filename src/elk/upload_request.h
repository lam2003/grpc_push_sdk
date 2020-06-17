#ifndef EDU_PUSH_SDK_UPLOAD_REQUEST_H
#define EDU_PUSH_SDK_UPLOAD_REQUEST_H

#include <common/config.h>
#include <common/utils.h>
#include <repo_version.h>

#include <json/json.h>

#include <deque>

namespace edu {

class ELKUploadItem {
  public:
    ELKUploadItem(const std::string system_time,
                  uint64_t          appid,
                  uint32_t          uid,
                  uint64_t          suid,
                  // uint64_t           gtype,
                  // uint64_t           gid,
                  const std::string& group_info,
                  const std::string& action,
                  int                code,
                  const std::string& msg)
    {
        this->system_time = system_time;
        this->appid       = appid;
        this->uid         = uid;
        this->suid        = suid;
        // this->gtype       = gtype;
        // this->gid         = gid;
        this->group_info = group_info;
        this->action     = action;
        this->code       = code;
        this->msg        = msg;
    }

  public:
    operator Json::Value() const
    {
        Json::Value root;
        root["sdk_version"]  = PUSH_SDK_VERSION;
        root["repo_version"] = REPO_VERSION;
        root["system_time"]  = system_time;
        root["appid"]        = static_cast<Json::UInt64>(appid);
        // root["gid"]          = static_cast<Json::UInt64>(gid);
        // root["gtype"]        = static_cast<Json::UInt64>(gtype);
        root["group_info"] = group_info;
        root["uid"]        = uid;
        root["suid"]       = static_cast<Json::UInt64>(suid);
        root["action"]     = action;
        root["code"]       = code;
        root["msg"]        = msg;
        root["platform"]   = Utils::GetPlatformName();

        return root;
    }

    operator std::string() 
    {
        Json::Value      root = *this;
        std::string      str;
        Json::FastWriter w;

        str = w.write(root);

        return str.substr(0, str.size() - 1);
    }

  public:
    std::string system_time;

    uint64_t appid;
    uint32_t uid;
    uint64_t suid;
    // uint64_t    gid;
    // uint64_t    gtype;
    std::string group_info;
    std::string action;
    int         code;
    std::string msg;
    std::string platform;
};

class ELKUploadRequest {
  public:
    operator std::string()
    {
        Json::Value root;
        root["project"]  = Config::Instance()->elk_project_name;
        root["region"]   = Config::Instance()->elk_region;
        root["logStore"] = Config::Instance()->elk_log_store;
        root["encode"]   = Config::Instance()->elk_encode;
        root["source"]   = Config::Instance()->elk_source;
        for (std::shared_ptr<ELKUploadItem>& item : contents) {
            root["content"].append(*item);
        }

        std::string      str;
        Json::FastWriter w;

        str = w.write(root);

        return str.substr(0, str.size() - 1);
    }

  public:
    std::deque<std::shared_ptr<ELKUploadItem>> contents;
};

}  // namespace edu

#endif