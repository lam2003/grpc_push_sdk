
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
    client_        = nullptr;
    thread_        = nullptr;
    run_           = false;
    logining_      = false;
    desc_          = "ok";
    code_          = RES_SUCCESS;
    user_          = nullptr;
}

int PushSDK::Initialize(uint32_t       uid,
                        uint64_t       appid,
                        uint64_t       appkey,
                        PushSDKEventCB cb_func,
                        void*          cb_args)
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
    if ((ret = client_->Initialize(
             uid_, Utils::GetSUID(uid_, get_user_terminal_type()))) !=
        PS_RET_SUCCESS) {
        log_e("client create channel failed. ret={}", PS_RET_SUCCESS);
        return ret;
    }

    run_    = true;
    thread_ = std::unique_ptr<std::thread>(new std::thread([this]() {
        while (run_) {
            {
                std::unique_lock<std::mutex> lock(cb_map_mux_);
                cb_map_cond_.wait_for(
                    lock, std::chrono::milliseconds(
                              Config::Instance()->call_check_timeout_interval));
                if (!run_) {
                    return;
                }
            }
            int64_t now = Utils::GetSteadyNanoSeconds();
            std::map<int64_t, std::shared_ptr<CallContext>>::iterator it;

            while (!cb_map_.empty()) {
                it = cb_map_.begin();

                int64_t                      call_time = it->first;
                std::shared_ptr<CallContext> ctx       = it->second;

                if (Utils::NanoSecondsToMilliSeconds(now - call_time) >=
                    Config::Instance()->call_timeout_interval) {
                    handle_timeout_response(ctx);
                    notify(ctx, PS_CB_EVENT_TIMEOUT, "timeout", RES_ETIMEOUT);
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

void PushSDK::NotifyChannelState(ChannelState state)
{
    log_d("channel state change to {}", channel_state_to_string(state));
    for (auto it = hdls_.begin(); it != hdls_.end(); it++) {
        if ((*it)->conn_state_cb) {
            (*it)->conn_state_cb(state == ChannelState::OK ?
                                     PushSDKConnState::PS_CONN_STATE_OK :
                                     PushSDKConnState::PS_CONN_STATE_NO_READY);
        }
    }
}

void PushSDK::OnConnected()
{
    relogin();
}

void PushSDK::OnFinish(std::shared_ptr<PushRegReq> last_req,
                       grpc::Status                status)
{
    switch (last_req->uri()) {
        case StreamURI::PPushGateWayLoginURI: {
            on_finish<LoginRequest, LoginResponse>(last_req, status);
            break;
        }
        case StreamURI::PPushGateWayLogoutURI: {
            on_finish<LogoutRequest, LogoutResponse>(last_req, status);
            break;
        }
        case StreamURI::PPushGateWayJoinGroupURI: {
            on_finish<JoinGroupRequest, JoinGroupResponse>(last_req, status);
            break;
        }
        case StreamURI::PPushGateWayLeaveGroupURI: {
            on_finish<LeaveGroupRequest, LeaveGroupResponse>(last_req, status);
            break;
        }
        default: break;
    }
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
        case StreamURI::PPushGateWayLeaveGroupResURI: {
            handle_response<LeaveGroupResponse>(msg);
            break;
        }
        case StreamURI::PPushGateWayNotifyToCloseURI: {
            handle_notify_to_close();
            break;
        }
        case StreamURI::PPushGateWayPushDataByGroupURI: {
            handle_group_message(msg);
            break;
        }
        case StreamURI::PPushGateWayPushDataByUidURI: {
            handle_user_message(msg);
            break;
        }
        default: break;
    }
}

void PushSDK::Destroy()
{
    if (!init_) {
        return;
    }

    cb_map_mux_.lock();
    run_ = false;
    cb_map_cond_.notify_one();
    cb_map_mux_.unlock();
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
                   bool                   is_sync,
                   PushSDKEventCB         cb_func,
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

    int64_t                     now = Utils::GetSteadyNanoSeconds();
    std::shared_ptr<PushRegReq> req =
        make_login_packet(uid_, appid_, appkey_, &user, now);
    if (!req) {
        ret = PS_RET_REQ_ENC_FAILED;
        log_e("encode login request packet failed. ret={}", ret);
        return ret;
    }

    PushSDKUserInfo* user_ptr = new PushSDKUserInfo;
    *user_ptr                 = user;
    user_.reset(user_ptr);

    logining_ = true;

    if (is_sync) {
        user_lock.unlock();
        ret = call_sync(PS_CB_TYPE_LOGIN, req, now);
    }
    else {
        call(PS_CB_TYPE_LOGIN, req, now, cb_func, cb_args);
    }

    return ret;
}

int PushSDK::Logout(bool is_sync, PushSDKEventCB cb_func, void* cb_args)
{
    int ret = PS_RET_SUCCESS;

    std::unique_lock<std::mutex> user_lock(user_mux_);

    if (!user_) {
        return ret;
    }

    int64_t                     now = Utils::GetSteadyNanoSeconds();
    std::shared_ptr<PushRegReq> req =
        make_logout_packet(uid_, appid_, appkey_, now);
    if (!req) {
        ret = PS_RET_REQ_ENC_FAILED;
        log_e("encode logout request packet failed. ret={}", ret);
        return ret;
    }

    // 清理登录信息
    remove_all_group_info();
    user_ = nullptr;

    if (is_sync) {
        user_lock.unlock();
        ret = call_sync(PS_CB_TYPE_LOGOUT, req, now);
    }
    else {
        call(PS_CB_TYPE_LOGOUT, req, now, cb_func, cb_args);
    }

    return ret;
}

int PushSDK::JoinGroup(const PushSDKGroupInfo& group,
                       bool                    is_sync,
                       PushSDKEventCB          cb_func,
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

    int64_t                     now = Utils::GetSteadyNanoSeconds();
    std::shared_ptr<PushRegReq> req =
        make_join_group_packet(uid_, group.gtype, group.gid, now);
    if (!req) {
        ret = PS_RET_REQ_ENC_FAILED;
        log_e("encode join group request packet failed. ret={}", ret);
        return ret;
    }

    groups_.insert(std::make_pair(group.gtype, group.gid));

    if (is_sync) {
        user_lock.unlock();
        ret =
            call_sync(PS_CB_TYPE_JOIN_GROUP, req, now, group.gtype, group.gid);
    }
    else {
        call(PS_CB_TYPE_JOIN_GROUP, req, now, cb_func, cb_args, group.gtype,
             group.gid);
    }

    return ret;
}

int PushSDK::LeaveGroup(const PushSDKGroupInfo& group,
                        bool                    is_sync,
                        PushSDKEventCB          cb_func,
                        void*                   cb_args)
{
    int ret = PS_RET_SUCCESS;

    std::unique_lock<std::mutex> user_lock(user_mux_);
    if (!user_) {
        ret = PS_RET_UNLOGIN;
        return ret;
    }

    if (!is_group_info_exists(group.gtype, group.gid)) {
        // 未加入该组直接返回成功
        return ret;
    }

    int64_t                     now = Utils::GetSteadyNanoSeconds();
    std::shared_ptr<PushRegReq> req =
        make_leave_group_packet(uid_, group.gtype, group.gid, now);
    if (!req) {
        ret = PS_RET_REQ_ENC_FAILED;
        log_e("encode leave group request packet failed. ret={}", ret);
        return ret;
    }

    remove_group_info(group.gtype, group.gid);

    if (is_sync) {
        user_lock.unlock();
        ret =
            call_sync(PS_CB_TYPE_LEAVE_GROUP, req, now, group.gtype, group.gid);
    }
    else {
        call(PS_CB_TYPE_LEAVE_GROUP, req, now, cb_func, cb_args, group.gtype,
             group.gid);
    }

    return ret;
}

void PushSDK::GetLastError(std::string& desc, int& code)
{
    desc = desc_;
    code = code_;
}

Handler* PushSDK::CreateHandler()
{
    Handler* hdl = new Handler;
    hdls_mux_.lock();
    hdls_.push_back(hdl);
    hdls_mux_.unlock();
    return hdl;
}

void PushSDK::DestroyHandler(Handler* hdl)
{
    std::unique_lock<std::mutex> lock(hdls_mux_);
    for (auto it = hdls_.begin(); it != hdls_.end(); it++) {
        if (*it == hdl) {
            delete hdl;
            hdls_.erase(it);
            return;
        }
    }
}

void PushSDK::AddUserMsgCBToHandler(Handler* hdl, PushSDKUserMsgCB cb)
{
    std::unique_lock<std::mutex> lock(hdls_mux_);
    for (auto it = hdls_.begin(); it != hdls_.end(); it++) {
        if (*it == hdl) {
            (*it)->user_msg_cb = cb;
            return;
        }
    }
}
void PushSDK::AddGroupMsgCBToHandler(Handler* hdl, PushSDKGroupMsgCB cb)
{
    std::unique_lock<std::mutex> lock(hdls_mux_);
    for (auto it = hdls_.begin(); it != hdls_.end(); it++) {
        if (*it == hdl) {
            (*it)->group_msg_cb = cb;
            return;
        }
    }
}
void PushSDK::AddConnStateCBToHandler(Handler* hdl, PushSDKConnStateCB cb)
{
    std::unique_lock<std::mutex> lock(hdls_mux_);
    for (auto it = hdls_.begin(); it != hdls_.end(); it++) {
        if (*it == hdl) {
            (*it)->conn_state_cb = cb;
            return;
        }
    }
}

void PushSDK::relogin(bool need_to_lock)
{
    std::unique_lock<std::mutex> user_lock(user_mux_);
    if (!user_ || logining_) {
        return;
    }

    client_->CleanQueue();

    int64_t                     now = Utils::GetSteadyNanoSeconds();
    std::shared_ptr<PushRegReq> req =
        make_login_packet(uid_, appid_, appkey_, user_.get(), now);
    if (!req) {
        user_ = nullptr;
        user_lock.unlock();
        log_e("encode login request packet failed");
        event_cb_(PS_CB_TYPE_LOGIN, PS_CB_EVENT_REQ_ENC_FAILED,
                  "inner relogin: LoginRequest packet serialize failed. you "
                  "should relogin manually",
                  event_cb_args_);
        return;
    }

    logining_ = true;
    call(PS_CB_TYPE_LOGIN, req, now, event_cb_, event_cb_args_, 0, 0, true,
         need_to_lock);
}

void PushSDK::rejoin_group(bool need_to_lock)
{
    std::unique_lock<std::mutex> user_lock(user_mux_);
    if (!user_ || groups_.empty()) {
        return;
    }

    int64_t                     now = Utils::GetSteadyNanoSeconds();
    std::shared_ptr<PushRegReq> req =
        make_join_group_packet(uid_, groups_, now);
    if (!req) {
        std::string dump_str = dump_all_group_info();
        remove_all_group_info();
        log_e("encode join group request packet failed");
        if (dump_str != "") {
            log_w("remove all group infos. dump={}", dump_str);
        }
        user_lock.unlock();
        event_cb_(PS_CB_TYPE_JOIN_GROUP, PS_CB_EVENT_REQ_ENC_FAILED,
                  "inner rejoin group : JoinGroupRequest packet serialize "
                  "failed. you "
                  "should rejoin all group manually",
                  event_cb_args_);
        return;
    }

    call(PS_CB_TYPE_JOIN_GROUP, req, now, event_cb_, event_cb_args_, 0, 0, true,
         need_to_lock);
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
            break;
        }
    }
}

std::string PushSDK::dump_all_group_info()
{
    if (groups_.empty()) {
        return "";
    }
    std::ostringstream oss;

    auto it = groups_.begin();
    for (; it != groups_.end(); it++) {
        oss << "<" << it->first << "," << it->second << "> ";
    }

    std::string str = oss.str();
    str             = str.substr(0, str.length() - 1);

    return str;
}

void PushSDK::remove_all_group_info()
{
    groups_.clear();
}

void PushSDK::call(PushSDKCBType               type,
                   std::shared_ptr<PushRegReq> msg,
                   int64_t                     now,
                   PushSDKEventCB              cb_func,
                   void*                       cb_args,
                   uint64_t                    gtype,
                   uint64_t                    gid,
                   bool                        is_retry,
                   bool                        need_to_lock)
{
    std::shared_ptr<CallContext> ctx = std::make_shared<CallContext>();
    ctx->cb_func                     = cb_func;
    ctx->cb_args                     = cb_args;
    ctx->type                        = type;
    ctx->gtype                       = gtype;
    ctx->gid                         = gid;
    ctx->is_retry                    = is_retry;

    if (need_to_lock) {
        cb_map_mux_.lock();
        cb_map_[now] = ctx;
        cb_map_mux_.unlock();
    }
    else {
        cb_map_[now] = ctx;
    }
    client_->Send(msg);
}

int PushSDK::call_sync(PushSDKCBType               type,
                       std::shared_ptr<PushRegReq> msg,
                       int64_t                     now,
                       uint64_t                    gtype,
                       uint64_t                    gid)
{
    std::shared_ptr<CallContext> ctx = std::make_shared<CallContext>();
    ctx->cb_func                     = nullptr;
    ctx->cb_args                     = nullptr;
    ctx->type                        = type;
    ctx->gtype                       = gtype;
    ctx->gid                         = gid;
    ctx->is_retry                    = false;

    cb_map_mux_.lock();
    cb_map_[now] = ctx;
    cb_map_mux_.unlock();

    client_->Send(msg);

    std::unique_lock<std::mutex> lock(ctx->mux);
    if (!ctx->call_done) {
        ctx->cond.wait_for(lock,
                           std::chrono::milliseconds(
                               Config::Instance()->call_timeout_interval * 2));
    }

    if (ctx->res == PS_CB_EVENT_OK) {
        desc_ = "ok";
        code_ = RES_SUCCESS;
        return PS_RET_SUCCESS;
    }
    else {
        desc_ = ctx->desc;
        code_ = ctx->code;
        if (ctx->res == PS_CB_EVENT_TIMEOUT) {
            return PS_RET_CALL_TIMEOUT;
        }
        return PS_RET_CALL_FAILED;
    }
}

void PushSDK::notify(std::shared_ptr<CallContext> ctx,
                     PushSDKCBEvent               res,
                     const std::string&           desc,
                     int                          code)
{
    if (!ctx->cb_func && !ctx->cb_args) {
        ctx->res       = res;
        ctx->desc      = desc;
        ctx->code      = code;
        ctx->call_done = true;
        ctx->cond.notify_all();
    }
    else {
        ctx->cb_func(ctx->type, res, desc.c_str(), ctx->cb_args);
    }
}

void PushSDK::handle_notify_to_close()
{
    std::unique_lock<std::mutex> user_lock(user_mux_);
    if (!user_) {
        return;
    }

    user_ = nullptr;
    user_lock.unlock();

    event_cb_(PS_CB_TYPE_LOGIN, PS_CB_EVENT_USER_KICKED_BY_SRV,
              "user be kicked by the server", event_cb_args_);
}

void PushSDK::handle_group_message(std::shared_ptr<PushData> msg)
{
    std::unique_lock<std::mutex> user_lock(user_mux_);
    if (!is_group_info_exists(msg->grouptype(), msg->groupid())) {
        // 用户已经退组，由于网络原因服务器没收到，这里再次向服务器发送退组信息
        client_->Send(
            make_leave_group_packet(uid_, msg->grouptype(), msg->groupid(), 0));
        return;
    }

    user_lock.unlock();

    std::unique_lock<std::mutex> lock(hdls_mux_);
    for (auto it = hdls_.begin(); it != hdls_.end(); it++) {
        if ((*it)->group_msg_cb) {
            (*it)->group_msg_cb(msg->grouptype(), msg->groupid(),
                                msg->msgdata().c_str(),
                                msg->msgdata().length());
        }
    }
}
void PushSDK::handle_user_message(std::shared_ptr<PushData> msg)
{
    std::unique_lock<std::mutex> user_lock(user_mux_);
    if (!user_) {
        // 用户已经登出，由于网络原因服务器没收到，这里再次向服务器发送登出信息
        client_->Send(make_logout_packet(uid_, appid_, appkey_, 0));
        return;
    }
    user_lock.unlock();

    std::unique_lock<std::mutex> lock(hdls_mux_);
    for (auto it = hdls_.begin(); it != hdls_.end(); it++) {
        if ((*it)->user_msg_cb) {
            (*it)->user_msg_cb(msg->msgdata().c_str(), msg->msgdata().length());
        }
    }
}

void PushSDK::handle_timeout_response(std::shared_ptr<CallContext> ctx)
{
    switch (ctx->type) {
        case PS_CB_TYPE_LOGIN: {
            logining_ = false;
            log_e("login timeout");
            if (ctx->is_retry) {
                relogin(false);
            }
            break;
        }

        case PS_CB_TYPE_JOIN_GROUP: {
            log_e("join group timeout");
            if (ctx->is_retry) {
                rejoin_group(false);
            }
            break;
        }

        case PS_CB_TYPE_LOGOUT: {
            log_e("logout timeout");
            break;
        }

        case PS_CB_TYPE_LEAVE_GROUP: {
            log_e("leave group timeout");
            break;
        }
        default: break;
    }
}
}  // namespace edu