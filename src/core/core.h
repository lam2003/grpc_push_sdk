#ifndef EDU_PUSH_SDK_CORE_H
#define EDU_PUSH_SDK_CORE_H

#include <common/err_code.h>
#include <common/singleton.h>
#include <core/client.h>

#include <push_sdk.h>

#include <condition_variable>
#include <memory>

namespace edu {

struct CallContext
{
    PushSDKCBType type;
    PushSDKCallCB cb_func;
    void*         cb_args;
};

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
    virtual int
                Login(const PushSDKUserInfo& user, PushSDKCallCB cb_func, void* cb_args);
    virtual int Logout(PushSDKCallCB cb_func, void* cb_args);
    virtual int JoinGroup(const PushSDKGroupInfo& group,
                          PushSDKCallCB           cb_func,
                          void*                   cb_args);
    virtual void OnChannelStateChange(ChannelState state) override;
    virtual void OnClientStatusChange(ClientStatus status) override;
    virtual void OnMessage(std::shared_ptr<PushData> msg) override;

  private:
    void handle_login_timeout(std::shared_ptr<CallContext> ctx);
    void handle_relogin_timeout(std::shared_ptr<CallContext> ctx);
    void handle_logout_timeout(std::shared_ptr<CallContext> ctx);
    void relogin();

    template <typename T> void handle_response(std::shared_ptr<PushData> msg)
    {
        T res;
        if (!res.ParseFromString(msg->msgdata())) {
            log_e("decode packet failed");
            event_cb_(PS_CB_INNER_ERR, PS_CALL_RES_DEC_FAILED,
                      "decode packet failed", event_cb_args_);
            return;
        }

        int64_t                      ts = std::stoll(res.context());
        std::shared_ptr<CallContext> ctx;

        {
            std::unique_lock<std::mutex> lock(map_mux_);
            if (cb_map_.find(ts) == cb_map_.end()) {
                log_e("logout response already timeout");
                return;
            }
            else {
                ctx = cb_map_[ts];
                cb_map_.erase(ts);
            }
        }

        PushSDKCBType type    = ctx->type;
        PushSDKCallCB cb_func = ctx->cb_func;
        void*         cb_args = ctx->cb_args;

        if (res.rescode() != RES_SUCCESS) {
            log_e("call failed. desc={}, code={}", res.errmsg(), res.rescode());
            cb_func(type, PS_CALL_RES_FAILE, res.errmsg().c_str(), cb_args);
        }
        else {
            log_d("call successfully");
            cb_func(type, PS_CALL_RES_OK, "ok", cb_args);
        }
    }

  private:
    bool                                            init_;
    uint32_t                                        uid_;
    uint64_t                                        appid_;
    uint64_t                                        appkey_;
    PushSDKCallCB                                   event_cb_;
    void*                                           event_cb_args_;
    std::mutex                                      user_mux_;
    std::unique_ptr<PushSDKUserInfo>                user_;
    std::shared_ptr<Client>                         client_;
    std::unique_ptr<std::thread>                    thread_;
    std::mutex                                      map_mux_;
    std::condition_variable                         map_cond_;
    std::atomic<bool>                               run_;
    std::atomic<bool>                               logining_;
    std::map<int64_t, std::shared_ptr<CallContext>> cb_map_;
};

}  // namespace edu

#endif