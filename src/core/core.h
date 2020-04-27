#ifndef EDU_PUSH_SDK_CORE_H
#define EDU_PUSH_SDK_CORE_H

#include <common/singleton.h>
#include <core/client.h>

#include <push_sdk.h>

#include <condition_variable>
#include <memory>

namespace edu {

class PushSDK : public Singleton<PushSDK>,
                public ChannelStateListener,
                public ClientStatusListener,
                public MessageHandler,
                public std::enable_shared_from_this<PushSDK> {
    friend class Singleton<PushSDK>;

  public:
    virtual ~PushSDK();

  protected:
    PushSDK();

  public:
    virtual int  Initialize(uint32_t      uid,
                            uint64_t      appid,
                            uint64_t      appkey,
                            PushSDKCallCB cb_func,
                            void*         cb_args);
    virtual void Destroy();
    virtual int  Login(const PushSDKUserInfo& user);
    virtual void OnChannelStateChange(ChannelState state) override;
    virtual void OnClientStatusChange(ClientStatus status) override;
    virtual void OnMessage(std::shared_ptr<PushData> msg) override;

  private:
    std::shared_ptr<PushRegReq> make_login_packet(int64_t now);
    void handle_login_response(std::shared_ptr<PushData> msg);
    void relogin();

  private:
    bool                             init_;
    uint32_t                         uid_;
    uint64_t                         appid_;
    uint64_t                         appkey_;
    PushSDKCallCB                    cb_func_;
    void*                            cb_args_;
    std::unique_ptr<PushSDKUserInfo> user_;
    std::shared_ptr<Client>          client_;
    std::unique_ptr<std::thread>     thread_;
    std::mutex                       mux_;
    std::condition_variable          cond_;
    std::atomic<bool>                run_;
    std::atomic<bool>                already_login_;
    std::map<int64_t, PushSDKCBType> cb_map_;
};

}  // namespace edu

#endif