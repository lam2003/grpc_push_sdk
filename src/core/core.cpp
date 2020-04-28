
#include <common/config.h>
#include <common/log.h>
#include <common/utils.h>
#include <core/core.h>
#include <core/packet.h>

namespace edu {

PushSDK::PushSDK()
{
    init_          = false;
    uid_           = 0;
    appid_         = 0;
    appkey_        = 0;
    event_cb_      = nullptr;
    event_cb_args_ = nullptr;
    user_          = nullptr;
    client_        = nullptr;
    thread_        = nullptr;
    run_           = false;
    logining_      = false;
}

int PushSDK::Initialize(uint32_t      uid,
                        uint64_t      appid,
                        uint64_t      appkey,
                        PushSDKCallCB cb_func,
                        void*         cb_args)
{
    int ret = PS_RET_SUCCESS;

    if (init_) {
        ret = PS_RET_ALREADY_INIT;
        log_w("sdk already initialized. ret={}", ret);
        return ret;
    }

    uid_           = uid;
    appid_         = appid;
    appkey_        = appkey;
    event_cb_      = cb_func;
    event_cb_args_ = cb_args;

    client_ = std::make_shared<Client>();
    client_->SetChannelStateListener(this->shared_from_this());
    client_->SetClientStatusListener(this->shared_from_this());
    client_->SetMessageHandler(this->shared_from_this());
    if ((ret = client_->Initialize(uid)) != PS_RET_SUCCESS) {
        log_e("client create channel failed. ret={}", PS_RET_SUCCESS);
        return ret;
    }

    run_    = true;
    thread_ = std::unique_ptr<std::thread>(new std::thread([this]() {
        while (run_) {
            std::unique_lock<std::mutex> lock(map_mux_);
            map_cond_.wait_for(
                lock, std::chrono::milliseconds(
                          Config::Instance()->call_check_timeout_interval));
            if (!run_) {
                return;
            }
            int64_t now = Utils::GetSteadyNanoSeconds();
            std::map<int64_t, std::shared_ptr<CallContext>>::iterator it;

            while (!cb_map_.empty()) {
                it = cb_map_.begin();

                int64_t                      call_time = it->first;
                std::shared_ptr<CallContext> ctx       = it->second;
                PushSDKCBType                type      = ctx->type;

                if (Utils::NanoSecondsToMilliSeconds(now - call_time) >=
                    Config::Instance()->call_timeout_interval) {
                    handle_timeout_response(type);
                    ctx->cb_func(PS_CB_LOGOUT, PS_CALL_TIMEOUT,
                                 "logout timeout. ignore it", ctx->cb_args);
                    cb_map_.erase(it);
                }
                else {
                    break;
                }
            }
        }
    }));

    init_ = true;
    return ret;
}

void PushSDK::OnChannelStateChange(ChannelState state)
{
    log_d("channel state change to {}", channel_state_to_string(state));

    if (state == ChannelState::NO_READY) {}
}

void PushSDK::OnClientStatusChange(ClientStatus status)
{
    log_d("client status change to {}", client_status_to_string(status));

    switch (status) {
        case ClientStatus::CONNECTED: {
            relogin();
            break;
        }
    }
}

void PushSDK::Destroy()
{
    if (!init_) {
        return;
    }

    run_ = false;
    map_cond_.notify_one();
    thread_->join();
    thread_.reset();
    thread_ = nullptr;

    client_->Destroy();
    client_.reset();
    client_ = nullptr;
    init_   = false;

    cb_map_.clear();
}

PushSDK::~PushSDK()
{
    Destroy();
}

int PushSDK::Login(const PushSDKUserInfo& user,
                   PushSDKCallCB          cb_func,
                   void*                  cb_args)
{
    int ret = PS_RET_SUCCESS;
    if (!init_) {
        ret = PS_RET_SDK_UNINIT;
        return ret;
    }

    std::unique_lock<std::mutex> user_lock(user_mux_);

    if (user_) {
        ret = PS_RET_ALREADY_LOGIN;
        log_w("sdk alreay login. ret={}", PS_RET_ALREADY_LOGIN);
        return ret;
    }

    PushSDKUserInfo* user_ptr = new PushSDKUserInfo;
    *user_ptr                 = user;

    user_.release();
    user_.reset(user_ptr);

    int64_t                     now = Utils::GetSteadyNanoSeconds();
    std::shared_ptr<PushRegReq> req =
        make_login_packet(uid_, appid_, appkey_, user_.get(), now);
    if (!req) {
        ret = PS_RET_REQ_ENC_FAILED;
        log_e("encode login request packet failed. ret={}", ret);
        return ret;
    }

    logining_ = true;
    call(PS_CB_LOGIN, req, now, cb_func, cb_args);

    return ret;
}

int PushSDK::Logout(PushSDKCallCB cb_func, void* cb_args)
{
    int ret = PS_RET_SUCCESS;

    std::unique_lock<std::mutex> user_lock(user_mux_);

    if (!user_) {
        return ret;
    }

    // 清理登录信息
    groups_.clear();
    user_.release();
    user_ = nullptr;

    int64_t                     now = Utils::GetSteadyNanoSeconds();
    std::shared_ptr<PushRegReq> req =
        make_logout_packet(uid_, appid_, appkey_, now);
    if (!req) {
        ret = PS_RET_REQ_ENC_FAILED;
        log_e("encode logout request packet failed. ret={}", ret);
        return ret;
    }

    call(PS_CB_LOGOUT, req, now, cb_func, cb_args);

    return ret;
}

int PushSDK::JoinGroup(const PushSDKGroupInfo& group,
                       PushSDKCallCB           cb_func,
                       void*                   cb_args)
{
    int ret = PS_RET_SUCCESS;

    std::unique_lock<std::mutex> user_lock(user_mux_);
    if (!user_) {
        ret = PS_RET_UNLOGIN;
        return ret;
    }

    if (is_group_info_exists(group.gtype, group.gid)) {
        ret = PS_RET_ALREADY_JOIN_GROUP;
        log_w("already join group. grouptype={}, groupid={}", group.gtype,
              group.gid);
        return ret;
    }

    groups_.insert(std::make_pair(group.gtype, group.gid));

    int64_t                     now = Utils::GetSteadyNanoSeconds();
    std::shared_ptr<PushRegReq> req =
        make_join_group_packet(uid_, group.gtype, group.gid, now);
    if (!req) {
        ret = PS_RET_REQ_ENC_FAILED;
        log_e("encode join group request packet failed. ret={}", ret);
        return ret;
    }

    call(PS_CB_JOIN_GROUP, req, now, cb_func, cb_args, group.gtype, group.gid);

    return ret;
}

void PushSDK::OnMessage(std::shared_ptr<PushData> msg)
{
    log_d("recv msg. uri={}", stream_uri_to_string(msg->uri()));
    switch (msg->uri()) {
        case StreamURI::PPushGateWayLoginResURI: {
            handle_response<LoginResponse>(msg);
            break;
        }
        case StreamURI::PPushGateWayLogoutResURI: {
            handle_response<LogoutResponse>(msg);
            break;
        }
        case StreamURI::PPushGateWayJoinGroupResURI: {
            handle_response<JoinGroupResponse>(msg);
            break;
        }
        default: break;
    }
}

void PushSDK::relogin()
{
    std::unique_lock<std::mutex> user_lock(user_mux_);
    if (!user_ || logining_) {
        return;
    }

    int64_t                     now = Utils::GetSteadyNanoSeconds();
    std::shared_ptr<PushRegReq> req =
        make_login_packet(uid_, appid_, appkey_, user_.get(), now);
    if (!req) {
        user_.release();
        user_.reset(nullptr);
        user_lock.unlock();
        log_e("encode login request packet failed");
        event_cb_(PS_CB_LOGIN, PS_CALL_REQ_ENC_FAILED,
                  "inner relogin: LoginRequest packet serialize failed. you "
                  "should relogin manually",
                  event_cb_args_);
        return;
    }

    logining_ = true;
    call(PS_CB_LOGIN, req, now, event_cb_, event_cb_args_);
}

void PushSDK::rejoin_group()
{
    std::unique_lock<std::mutex> user_lock(user_mux_);
    if (groups_.empty()) {
        return;
    }

    int64_t                     now = Utils::GetSteadyNanoSeconds();
    std::shared_ptr<PushRegReq> req =
        make_join_group_packet(uid_, groups_, now);
    if (!req) {
        groups_.clear();
        user_lock.unlock();
        log_e("encode join group request packet failed");
        event_cb_(PS_CB_JOIN_GROUP, PS_CALL_REQ_ENC_FAILED,
                  "inner rejoin group : JoinGroupRequest packet serialize "
                  "failed. you "
                  "should rejoin all group manually",
                  event_cb_args_);
        return;
    }

    call(PS_CB_JOIN_GROUP, req, now, event_cb_, event_cb_args_);
}

bool PushSDK::is_group_info_exists(uint64_t gtype, uint64_t gid)
{
    auto sit = groups_.equal_range(gtype);

    for (auto p = sit.first; p != sit.second; p++) {
        if (p->second == gid) {
            return true;
        }
    }

    return false;
}

void PushSDK::remove_group_info(uint64_t gtype, uint64_t gid)
{
    auto sit = groups_.equal_range(gtype);

    for (auto p = sit.first; p != sit.second; p++) {
        if (p->second == gid) {
            groups_.erase(p);
        }
    }
}

void PushSDK::call(PushSDKCBType               type,
                   std::shared_ptr<PushRegReq> msg,
                   int64_t                     now,
                   PushSDKCallCB               cb_func,
                   void*                       cb_args,
                   uint64_t                    gtype,
                   uint64_t                    gid)
{
    std::shared_ptr<CallContext> ctx = std::make_shared<CallContext>();
    ctx->cb_func                     = cb_func;
    ctx->cb_args                     = cb_args;
    ctx->type                        = type;
    ctx->gtype                       = gtype;
    ctx->gid                         = gid;

    map_mux_.lock();
    cb_map_[now] = ctx;
    map_mux_.unlock();

    client_->Send(msg);
}

void PushSDK::handle_timeout_response(PushSDKCBType type)
{
    switch (type) {
        case PS_CB_LOGIN: logining_ = false; break;
        default: break;
    }
}
}  // namespace edu