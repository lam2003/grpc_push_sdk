#ifndef EDU_PUSH_SDK_CORE_H
#define EDU_PUSH_SDK_CORE_H

#include <common/singleton.h>

namespace edu {

class Client;

class PushSDK : public Singleton<PushSDK> {
    friend class Singleton<PushSDK>;

  public:
    virtual ~PushSDK();
    virtual int  Initialize();
    virtual void Destroy();

  protected:
    PushSDK();

  private:
    bool init_;

    std::shared_ptr<Client> client_;
};

}  // namespace edu

#endif