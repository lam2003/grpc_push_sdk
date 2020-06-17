#ifndef EDU_PUSH_SDK_UPLOAD_REQUEST_H
#define EDU_PUSH_SDK_UPLOAD_REQUEST_H

#include <common/config.h>

#include <jsoncpp/json.h>

namespace edu {

class ELKUploadItem {
  public:
  public:
    std::string sdk_version;
    uint64_t    appid;
    union
    {
        uint32_t uid;
        uint64_t suid;
        uint64_t gid;
        uint64_t gtype;
    } info;
    std::string uri;
    int         code;
    std::string msg;
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
    }

  public:
    std::vector<ELKUploadItem> items;
};

}  // namespace edu

#endif