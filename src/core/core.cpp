
#include <common/err_code.h>
#include <common/log.h>
#include <common/utils.h>
#include <core/core.h>

#define CHECK_TIMEOUT_INTERVAL 500  // ms
#define CALL_TIMEOUT 2000000        // us

namespace edu {

static UserTerminalType get_user_terminal_type()
{
    UserTerminalType utt;
    switch (Utils::GetTerminalType()) {
        case TerminalType::UNKNOWN: utt = UserTerminalType::UTT_UNKNOWN; break;
        case TerminalType::WINDOWS_32:
            utt = UserTerminalType::UTT_DESKTOP;
            break;
        case TerminalType::WINDOWS_64:
            utt = UserTerminalType::UTT_DESKTOP;
            break;
        case TerminalType::IOS: utt = UserTerminalType::UTT_IPHONE; break;
        case TerminalType::ANDROID: utt = UserTerminalType::UTT_ANDROID; break;
        case TerminalType::IOS_SIMULATOR:
            utt = UserTerminalType::UTT_IPHONE;
            break;
        case TerminalType::LINUX: utt = UserTerminalType::UTT_SERVER; break;
        case TerminalType::UNIX: utt = UserTerminalType::UTT_SERVER; break;
        case TerminalType::MAC: utt = UserTerminalType::UTT_SERVER; break;
        case TerminalType::APPLE_UNKNOWN:
            utt = UserTerminalType::UTT_UNKNOWN;
            break;
        case TerminalType::POSIX_UNKNOWN:
            utt = UserTerminalType::UTT_UNKNOWN;
            break;
        default: utt = UserTerminalType::UTT_UNKNOWN; break;
    }

    return utt;
}

PushSDK::PushSDK()
{
    init_           = false;
    uid_            = 0;
    appid_          = 0;
    appkey_         = 0;
    cb_func_        = nullptr;
    cb_args_        = nullptr;
    user_           = nullptr;
    client_         = nullptr;
    thread_         = nullptr;
    run_            = false;
    logining_       = false;
    login_manually_ = true;
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

    uid_     = uid;
    appid_   = appid;
    appkey_  = appkey;
    cb_func_ = cb_func;
    cb_args_ = cb_args;

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
                lock, std::chrono::milliseconds(CHECK_TIMEOUT_INTERVAL));
            if (!run_) {
                return;
            }
            int64_t now = Utils::GetSteadyMicroSeconds();
            std::map<int64_t, PushSDKCBType>::iterator it;
            while (!cb_map_.empty()) {
                it = cb_map_.begin();

                int64_t       call_time = it->first;
                PushSDKCBType type      = it->second;

                if (now - call_time >= CALL_TIMEOUT) {
                    if (type == PS_CB_LOGIN || type == PS_CB_RELOGIN) {
                        logining_ = false;

                        if (type == PS_CB_LOGIN) {
                            login_manually_ = false;
                        }

                        cb_func_(type, PS_CALL_TIMEOUT,
                                 "login timeout and we will inner retry",
                                 cb_args_);
                    }
                    else if (type == PS_CB_LOGOUT) {
                        cb_func_(type, PS_CALL_TIMEOUT,
                                 "logout timeout. ignore it", cb_args_);
                    }
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

std::shared_ptr<PushRegReq> PushSDK::make_login_packet(int64_t now)
{
    LoginRequest login_req;
    login_req.set_uid(uid_);
    login_req.set_suid(Utils::GetSUID(uid_, get_user_terminal_type()));
    login_req.set_appid(std::to_string(appid_));
    login_req.set_appkey(appkey_);
    login_req.set_termnialtype(get_user_terminal_type());
    login_req.set_account(std::string(user_->account));
    login_req.set_password(std::string(user_->passwd));
    login_req.set_cookie(std::string(user_->token));
    login_req.set_context(std::to_string(now));

    std::string msg_data;
    if (!login_req.SerializeToString(&msg_data)) {
        log_e("LoginRequest packet serialize failed");
        return nullptr;
    }

    std::shared_ptr<PushRegReq> req = std::make_shared<PushRegReq>();
    req->set_uri(StreamURI::PPushGateWayLoginURI);
    req->set_msgdata(msg_data);

    return req;
}

std::shared_ptr<PushRegReq> PushSDK::make_logout_packet(int64_t now)
{
    LogoutRequest logout_req;
    logout_req.set_appid(std::to_string(appid_));
    logout_req.set_uid(uid_);
    logout_req.set_suid(Utils::GetSUID(uid_, get_user_terminal_type()));
    logout_req.set_context(std::to_string(now));
    logout_req.set_appkey(appkey_);
    logout_req.set_termnialtype(get_user_terminal_type());

    std::string msg_data;
    if (!logout_req.SerializeToString(&msg_data)) {
        log_e("LogoutRequest packet serialize failed");
        return nullptr;
    }

    std::shared_ptr<PushRegReq> req = std::make_shared<PushRegReq>();
    req->set_uri(StreamURI::PPushGateWayLogoutURI);
    req->set_msgdata(msg_data);

    return req;
}

void PushSDK::relogin()
{
    std::unique_lock<std::mutex> user_lock(user_mux_);
    if (!user_ || logining_) {
        return;
    }

    int64_t                     now = Utils::GetSteadyMicroSeconds();
    std::shared_ptr<PushRegReq> req = make_login_packet(now);
    if (!req) {
        user_.release();
        user_.reset(nullptr);
        user_lock.unlock();
        log_e("encode login request packet failed");
        cb_func_(PS_CB_RELOGIN, PS_CALL_RELOGIN_REQ_ENC_FAILED,
                 "inner relogin: LoginRequest packet serialize failed. you "
                 "should relogin manually",
                 cb_args_);
        return;
    }

    user_lock.unlock();

    map_mux_.lock();
    cb_map_[now] = PushSDKCBType::PS_CB_RELOGIN;
    map_mux_.unlock();

    logining_ = true;
    client_->Send(req);
}

int PushSDK::Login(const PushSDKUserInfo& user)
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

    int64_t                     now = Utils::GetSteadyMicroSeconds();
    std::shared_ptr<PushRegReq> req = make_login_packet(now);
    if (!req) {
        ret = PS_RET_LOGIN_REQ_ENC_FAILED;
        log_e("encode login request packet failed. ret={}", ret);
        return ret;
    }

    user_lock.unlock();

    logining_       = true;
    login_manually_ = true;

    map_mux_.lock();
    cb_map_[now] = PushSDKCBType::PS_CB_LOGIN;
    map_mux_.unlock();
    client_->Send(req);

    return ret;
}

int PushSDK::Logout()
{
    int ret = PS_RET_SUCCESS;

    std::unique_lock<std::mutex> user_lock(user_mux_);

    if (!user_) {
        return ret;
    }

    // 清理登录信息
    user_.release();
    user_ = nullptr;

    user_lock.unlock();

    int64_t                     now = Utils::GetSteadyMicroSeconds();
    std::shared_ptr<PushRegReq> req = make_logout_packet(now);
    if (!req) {
        ret = PS_RET_LOGOUT_REQ_ENC_FAILED;
        log_e("encode logout request packet failed. ret={}", ret);
        return ret;
    }

    map_mux_.lock();
    cb_map_[now] = PushSDKCBType::PS_CB_LOGOUT;
    map_mux_.unlock();
    client_->Send(req);

    return ret;
}

void PushSDK::OnMessage(std::shared_ptr<PushData> msg)
{
    switch (msg->uri()) {
        case StreamURI::PPushGateWayLoginResURI: {
            log_d("recv msg. uri=PPushGateWayLoginResURI");
            handle_login_response(msg);
            break;
        }
        case StreamURI::PPushGateWayLogoutResURI: {
            log_d("recv msg. uri=PPushGateWayLogoutResURI");
            handle_logout_response(msg);
            break;
        }
    }
}

void PushSDK::handle_login_response(std::shared_ptr<PushData> msg)
{
    logining_ = false;

    LoginResponse res;
    // 登录错误时，清理原来的登录信息
    if (!res.ParseFromString(msg->msgdata())) {
        user_mux_.lock();
        user_.reset();
        user_ = nullptr;
        user_mux_.unlock();

        if (login_manually_) {
            login_manually_ = false;
            log_e("decode login response packet failed");
            cb_func_(PS_CB_LOGIN, PS_CALL_LOGIN_RES_DEC_FAILED,
                     "decode login response packet failed", cb_args_);
        }
        else {
            log_e("inner relogin: decode login response packet failed");
            cb_func_(PS_CB_RELOGIN, PS_CALL_RELOGIN_RES_DEC_FAILED,
                     "inner relogin: decode login response packet failed. you "
                     "should relogin manually",
                     cb_args_);
        }
        return;
    }

    int64_t       ts = std::stoll(res.context());
    PushSDKCBType cb_type;

    {
        std::unique_lock<std::mutex> lock(map_mux_);
        if (cb_map_.find(ts) == cb_map_.end()) {
            log_e("login response already timeout");
            return;
        }
        else {
            cb_type = cb_map_[ts];
            cb_map_.erase(ts);
        }
    }

    if (res.rescode() != RES_SUCCESS) {
        user_mux_.lock();
        user_.reset();
        user_ = nullptr;
        user_mux_.unlock();

        if (cb_type == PS_CB_LOGIN) {
            login_manually_ = false;
            log_e("user login failed. desc={}, code={}", res.errmsg(),
                  res.rescode());
            cb_func_(cb_type, PS_CALL_RES_FAILE, res.errmsg().c_str(),
                     cb_args_);
        }
        else {
            log_e("inner relogin: user login failed. desc={}, code={}",
                  res.errmsg(), res.rescode());
            cb_func_(cb_type, PS_CALL_RES_FAILE,
                     ("inner relogin: " + res.errmsg() +
                      ". you should relogin manually")
                         .c_str(),
                     cb_args_);
        }
    }
    else {
        if (cb_type == PS_CB_LOGIN) {
            login_manually_ = false;
            log_d("user login successfully");
            cb_func_(cb_type, PS_CALL_RES_OK, "ok", cb_args_);
        }
        else {
            log_d("inner relogin: user login successfully");
            cb_func_(cb_type, PS_CALL_RES_OK, "inner relogin: ok", cb_args_);
        }
    }
}

void PushSDK::handle_logout_response(std::shared_ptr<PushData> msg)
{
    LogoutResponse res;
    if (!res.ParseFromString(msg->msgdata())) {
        log_e("decode logout response packet failed");
        cb_func_(PS_CB_LOGOUT, PS_CALL_LOGOUT_RES_DEC_FAILED,
                 "decode logout response packet failed", cb_args_);
        return;
    }

    int64_t ts = std::stoll(res.context());
    {
        std::unique_lock<std::mutex> lock(map_mux_);
        if (cb_map_.find(ts) == cb_map_.end()) {
            log_e("logout response already timeout");
            return;
        }
        else {
            cb_map_.erase(ts);
        }
    }

    if (res.rescode() != RES_SUCCESS) {
        log_e("user logout failed. desc={}, code={}", res.errmsg(),
              res.rescode());
        cb_func_(PS_CB_LOGOUT, PS_CALL_RES_FAILE, res.errmsg().c_str(),
                 cb_args_);
    }
    else {
        log_d("user logout successfully");
        cb_func_(PS_CB_LOGOUT, PS_CALL_RES_OK, "ok", cb_args_);
    }
}

}  // namespace edu