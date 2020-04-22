#ifndef EDU_PUSH_SDK_CORE_H
#define EDU_PUSH_SDK_CORE_H

#include <common/singleton.h>

namespace edu {

class PushSDK : public Singleton<PushSDK> {
    friend class Singleton<PushSDK>;

  public:
    virtual ~PushSDK();
    virtual int Initialize();

  protected:
    PushSDK();

  private:
    uint64_t appid_;
    uint64_t uid_;
    uint64_t app_key_;
    bool     init_;
};

}  // namespace edu

#endif