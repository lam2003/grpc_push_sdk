#ifndef EDU_PUSH_SDK_CORE_H
#define EDU_PUSH_SDK_CORE_H

#include <common/singleton.h>
#include <core/client.h>

#include <memory>

namespace edu {

class PushSDK : public Singleton<PushSDK>,
                public ChannelStateListener,
                public ClientStatusListener,
                public std::enable_shared_from_this<PushSDK> {
    friend class Singleton<PushSDK>;

  public:
    virtual ~PushSDK();

  protected:
    PushSDK();

  public:
    virtual int  Initialize(uint32_t uid);
    virtual void Destroy();
    virtual void OnChannelStateChange(ChannelState state) override;
    virtual void OnClientStatusChange(ClientStatus status) override;

  private:
    bool init_;

    std::shared_ptr<Client> client_;
};

}  // namespace edu

#endif