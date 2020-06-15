#ifndef EDU_PUSH_SDK_CORE_H
#define EDU_PUSH_SDK_CORE_H

#include <common/err_code.h>
#include <common/singleton.h>
#include <core/client.h>

#include <push_sdk.h>

#include <condition_variable>
#include <memory>
#include <sstream>

namespace edu {

struct Handler
{
    Handler()
    {
        user_msg_cb   = nullptr;
        group_msg_cb  = nullptr;
        conn_state_cb = nullptr;
    }

    PushSDKUserMsgCB   user_msg_cb;
    PushSDKGroupMsgCB  group_msg_cb;
    PushSDKConnStateCB conn_state_cb;
};

struct CallContext
{
    CallContext()
    {
        call_done = false;
        res       = PS_CB_EVENT_OK;
        desc      = "timeout";
        code      = RES_ETIMEOUT;
    }

    PushSDKCBType  type;
    PushSDKEventCB cb_func;
    void*          cb_args;

    uint64_t gtype;
    uint64_t gid;
    bool     is_retry;

    // 同步接口使用
    std::mutex              mux;
    std::condition_variable cond;
    bool                    call_done;
    PushSDKCBEvent          res;
    std::string             desc;
    int                     code;
};

class PushSDK : public Singleton<PushSDK>,
                public ChannelStateListener,
                public StreamStatusListener,
                public MessageHandler,
                public std::enable_shared_from_this<PushSDK> {
    friend class Singleton<PushSDK>;

  public:
    virtual ~PushSDK();

  protected:
    PushSDK();

  public:
    virtual int  Initialize(uint32_t       uid,
                            uint64_t       appid,
                            uint64_t       appkey,
                            PushSDKEventCB cb_func,
                            void*          cb_args);
    virtual void Destroy();
    virtual int  Login(const PushSDKUserInfo& user,
                       bool                   is_sync = true,
                       PushSDKEventCB         cb_func = nullptr,
                       void*                  cb_args = nullptr);

    virtual int Logout(bool           is_sync = true,
                       PushSDKEventCB cb_func = nullptr,
                       void*          cb_args = nullptr);
    virtual int JoinGroup(const PushSDKGroupInfo& group,
                          bool                    is_sync = true,
                          PushSDKEventCB          cb_func = nullptr,
                          void*                   cb_args = nullptr);
    virtual int LeaveGroup(const PushSDKGroupInfo& group,
                           bool                    is_sync = true,
                           PushSDKEventCB          cb_func = nullptr,
                           void*                   cb_args = nullptr);

    virtual void GetLastError(std::string& desc, int& code);

    virtual Handler* CreateHandler();
    virtual void     DestroyHandler(Handler* hdl);
    virtual void     AddUserMsgCBToHandler(Handler* hdl, PushSDKUserMsgCB cb);
    virtual void     AddGroupMsgCBToHandler(Handler* hdl, PushSDKGroupMsgCB cb);
    virtual void AddConnStateCBToHandler(Handler* hdl, PushSDKConnStateCB cb);

    virtual void NotifyChannelState(ChannelState state) override;
    virtual void OnConnected() override;
    virtual void OnFinish(std::shared_ptr<PushRegReq> last_req,
                          grpc::Status                status) override;
    virtual void OnMessage(std::shared_ptr<PushData> msg) override;

  private:
    bool        is_group_info_exists(uint64_t gtype, uint64_t gid);
    void        remove_group_info(uint64_t gtype, uint64_t gid);
    std::string dump_all_group_info();
    void        remove_all_group_info();

    void call(PushSDKCBType               type,
              std::shared_ptr<PushRegReq> msg,
              int64_t                     now,
              PushSDKEventCB              cb_func,
              void*                       cb_args,
              uint64_t                    gtype        = 0,
              uint64_t                    gid          = 0,
              bool                        is_retry     = false,
              bool                        need_to_lock = true);

    int  call_sync(PushSDKCBType               type,
                   std::shared_ptr<PushRegReq> msg,
                   int64_t                     now,
                   uint64_t                    gtype = 0,
                   uint64_t                    gid   = 0);
    void notify(std::shared_ptr<CallContext> ctx,
                PushSDKCBEvent               res,
                const std::string&           desc,
                int                          code);

    void relogin(bool need_to_lock = true);
    void rejoin_group(bool need_to_lock = true);

    void handle_timeout_response(std::shared_ptr<CallContext> ctx);
    void handle_notify_to_close();
    void handle_group_message(std::shared_ptr<PushData> msg);
    void handle_user_message(std::shared_ptr<PushData> msg);

    template <typename T>
    void handle_failed_response(const T& res, std::shared_ptr<CallContext> ctx)
    {
        if (std::is_same<T, LoginResponse>::value) {
            // 登录失败，清理登录信息
            log_e("login failed. desc={}, code={}", res.errmsg(),
                  res.rescode());
            // 清除正在登录状态
            logining_ = false;
            user_mux_.lock();
            std::string dump_str = dump_all_group_info();
            if (dump_str != "") {
                log_w("remove all group infos. dump={}", dump_str);
            }
            remove_all_group_info();
            user_ = nullptr;
            user_mux_.unlock();
        }
        else if (std::is_same<T, LogoutResponse>::value) {
            log_e("logout failed. desc={}, code={}", res.errmsg(),
                  res.rescode());
        }
        else if (std::is_same<T, JoinGroupResponse>::value) {
            log_e("join group failed. desc={}, code={}", res.errmsg(),
                  res.rescode());

            user_mux_.lock();
            // 进组失败，清理组信息
            if (ctx->gtype != 0 && ctx->gid != 0) {
                // SDK外部调用JoinGroup，仅清除单个
                log_w("remove gtype={}, gid={}", ctx->gtype, ctx->gid);
                remove_group_info(ctx->gtype, ctx->gid);
            }
            else {
                std::string dump_str = dump_all_group_info();
                if (dump_str != "") {
                    log_w("remove all group infos. dump={}", dump_str);
                }
                // SDK内部重新进组，全量清除
                remove_all_group_info();
            }
            user_mux_.unlock();
        }
        else if (std::is_same<T, LeaveGroupResponse>::value) {
            log_e("leave group failed. desc={}, code={}", res.errmsg(),
                  res.rescode());
        }
        else {
            // ignore
        }
    }

    template <typename T>
    void handle_success_response(std::shared_ptr<CallContext> ctx)
    {
        if (std::is_same<T, LoginResponse>::value) {
            log_i("login successfully");
            // 清除正在登录状态
            logining_ = false;
            // 重新进组
            if (ctx->is_retry) {
                rejoin_group();
            }
        }
        else if (std::is_same<T, LogoutResponse>::value) {
            log_i("logout successfully");
        }
        else if (std::is_same<T, JoinGroupResponse>::value) {
            if (ctx->gtype == 0 && ctx->gid == 0) {
                //全量进组
                std::string dump_str = dump_all_group_info();
                if (dump_str != "") {
                    log_i("join group successfully. dump={}", dump_str);
                }
            }
            else {
                log_i("join group successfully. gtype={}, gid={}", ctx->gtype,
                      ctx->gid);
            }
        }
        else if (std::is_same<T, LeaveGroupResponse>::value) {
            log_i("leave group successfully. gtype={}, gid={}", ctx->gtype,
                  ctx->gid);
        }
        else {
            // ignore
        }
    }

    template <typename T1, typename T2>
    void on_finish(std::shared_ptr<PushRegReq> last_req, grpc::Status status)
    {
        T1 req;
        assert(req.ParseFromString(last_req->msgdata()));

        int64_t                      ts = std::stoll(req.context());
        std::shared_ptr<CallContext> ctx;

        {
            std::unique_lock<std::mutex> lock(cb_map_mux_);
            if (ts == 0) {
                // 这个回复属于客户端重新发送登出、离组请求，直接返回
                return;
            }
            else if (cb_map_.find(ts) == cb_map_.end()) {
                return;
            }
            else {
                ctx = cb_map_[ts];
                cb_map_.erase(ts);
            }
        }

        T2 res;
        res.set_errmsg(status.error_message());
        res.set_rescode(status.error_code());
        if (status.error_code() != grpc::StatusCode::OK) {
            handle_failed_response<T2>(res, ctx);
            notify(ctx, PS_CB_EVENT_FAILED, res.errmsg().c_str(),
                   res.rescode());
        }
        else {
            handle_success_response<T2>(ctx);
            notify(ctx, PS_CB_EVENT_OK, "ok", 0);
        }
    }

    template <typename T> void handle_response(std::shared_ptr<PushData> msg)
    {
        T res;
        if (!res.ParseFromString(msg->msgdata())) {
            log_e("decode packet failed");
            {
                std::unique_lock<std::mutex> lock(event_cb_mux_);
                event_cb_pctxs_.emplace_back(std::make_shared<EventCBContext>(
                    PS_CB_TYPE_INNER_ERR, PS_CB_EVENT_RES_DEC_FAILED,
                    "decode packet failed"));
                event_cb_cond_.notify_one();
            }
            return;
        }

        int64_t                      ts = std::stoll(res.context());
        std::shared_ptr<CallContext> ctx;

        {
            std::unique_lock<std::mutex> lock(cb_map_mux_);
            if (ts == 0) {
                // 这个回复属于客户端重新发送登出、离组请求，直接返回
                return;
            }
            else if (cb_map_.find(ts) == cb_map_.end()) {
                return;
            }
            else {
                ctx = cb_map_[ts];
                cb_map_.erase(ts);
            }
        }

        if (res.rescode() != RES_SUCCESS) {
            handle_failed_response<T>(res, ctx);
            notify(ctx, PS_CB_EVENT_FAILED, res.errmsg().c_str(),
                   res.rescode());
        }
        else {
            handle_success_response<T>(ctx);
            notify(ctx, PS_CB_EVENT_OK, "ok", 0);
        }
    }

  private:
    struct EventCBContext
    {
        EventCBContext(PushSDKCBType     type,
                       PushSDKCBEvent    res,
                       const std::string desc)
        {
            this->type = type;
            this->res  = res;
            this->desc = desc;
        }
        PushSDKCBType  type;
        PushSDKCBEvent res;
        std::string    desc;
    };

  private:
    bool                              init_;
    uint32_t                          uid_;
    uint64_t                          appid_;
    uint64_t                          appkey_;
    PushSDKEventCB                    event_cb_;
    void*                             event_cb_arg_;
    std::shared_ptr<Client>           client_;
    std::unique_ptr<std::thread>      thread_;
    std::atomic<bool>                 run_;
    std::atomic<bool>                 logining_;
    std::string                       desc_;
    int                               code_;
    std::multimap<uint64_t, uint64_t> groups_;

    std::vector<Handler*> hdls_;
    std::mutex            hdls_mux_;

    std::unique_ptr<PushSDKUserInfo> user_;
    std::mutex                       user_mux_;

    std::map<int64_t, std::shared_ptr<CallContext>> cb_map_;
    std::mutex                                      cb_map_mux_;
    std::condition_variable                         cb_map_cond_;

    std::unique_ptr<std::thread>                event_cb_thread_;
    std::deque<std::shared_ptr<EventCBContext>> event_cb_pctxs_;
    std::condition_variable                     event_cb_cond_;
    std::mutex                                  event_cb_mux_;
    std::thread::id                             event_cb_thread_id_;
    std::atomic<bool>                           event_cb_thread_quit_flag_;
};

}  // namespace edu

#endif