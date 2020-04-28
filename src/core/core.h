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

    //记录进组信息
    uint64_t gtype;
    uint64_t gid;
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
    virtual int  Logout(PushSDKCallCB cb_func, void* cb_args);
    virtual int  JoinGroup(const PushSDKGroupInfo& group,
                           PushSDKCallCB           cb_func,
                           void*                   cb_args);
    virtual void OnChannelStateChange(ChannelState state) override;
    virtual void OnClientStatusChange(ClientStatus status) override;
    virtual void OnMessage(std::shared_ptr<PushData> msg) override;

  private:
    bool is_group_info_exists(uint64_t gtype, uint64_t gid);
    void remove_group_info(uint64_t gtype, uint64_t gid);
    void call(PushSDKCBType               type,
              std::shared_ptr<PushRegReq> msg,
              int64_t                     now,
              PushSDKCallCB               cb_func,
              void*                       cb_args,
              uint64_t                    gtype = 0,
              uint64_t                    gid   = 0);
    void relogin();
    void rejoin_group();

    void handle_timeout_response(PushSDKCBType type);

    template <typename T>
    void handle_failed_response(std::shared_ptr<CallContext> ctx)
    {
        if (std::is_same<T, LoginResponse>::value) {
            // 登录失败，清理登录信息
            // 清除正在登录状态
            logining_ = false;
            user_mux_.lock();
            groups_.clear();
            user_.release();
            user_ = nullptr;
            user_mux_.unlock();
        }
        else if (std::is_same<T, JoinGroupResponse>::value) {
            user_mux_.lock();
            // 进组失败，清理组信息

            if (ctx->gtype != 0 && ctx->gid != 0) {
                // SDK外部调用JoinGroup，仅清除单个
                remove_group_info(ctx->gtype, ctx->gid);
            }
            else {
                // SDK内部重新进组，全量清除
                groups_.clear();
            }
            user_mux_.unlock();
        }
        else {
            // ignore
        }
    }

    template <typename T>
    void handle_success_response(std::shared_ptr<CallContext> ctx)
    {
        if (std::is_same<T, LoginResponse>::value) {
            // 清除正在登录状态
            logining_ = false;
            // 重新进组
            rejoin_group();
        }
        else {
            // ignore
        }
    }

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
                log_w("response already timeout. uri={}",
                      stream_uri_to_string(msg->uri()));
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
            handle_failed_response<T>(ctx);
            log_e("call failed. desc={}, code={}", res.errmsg(), res.rescode());
            cb_func(type, PS_CALL_RES_FAILE, res.errmsg().c_str(), cb_args);
        }
        else {
            handle_success_response<T>(ctx);
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
    std::multimap<uint64_t, uint64_t>               groups_;
};

}  // namespace edu

#endif