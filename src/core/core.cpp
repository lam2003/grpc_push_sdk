
#include <common/log.h>
#include <common/utils.h>
#include <core/core.h>

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
    init_ = false;
}

int PushSDK::Initialize(uint32_t uid, uint64_t appid, uint64_t appkey)
{
    int ret = PS_RET_SUCCESS;

    if (init_) {
        ret = PS_RET_ALREADY_INIT;
        log_w("sdk already initialized. ret={}", ret);
        return ret;
    }

    uid_    = uid;
    appid_  = appid;
    appkey_ = appkey;

    client_ = std::make_shared<Client>();
    client_->SetChannelStateListener(this->shared_from_this());
    client_->SetClientStatusListener(this->shared_from_this());
    client_->SetMessageHandler(this->shared_from_this());
    if ((ret = client_->Initialize(uid)) != PS_RET_SUCCESS) {
        log_e("client create channel failed. ret={}", PS_RET_SUCCESS);
        return ret;
    }

    init_ = true;
    return ret;
}

void PushSDK::OnChannelStateChange(ChannelState state)
{
    log_d("channel state change to {}", channel_state_to_string(state));
}

void PushSDK::OnClientStatusChange(ClientStatus status)
{
    log_d("client status change to {}", client_status_to_string(status));

    switch (status) {
        case ClientStatus::CONNECTED: {
        }
    }
}

void PushSDK::Destroy()
{
    if (!init_) {
        return;
    }

    client_->Destroy();
    client_.reset();
    client_ = nullptr;
    init_   = false;
}

PushSDK::~PushSDK()
{
    Destroy();
}

std::shared_ptr<PushRegReq> PushSDK::make_login_packet()
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

int PushSDK::Login(const PushSDKUserInfo& user)
{
    int ret = PS_RET_SUCCESS;
    if (!init_) {
        ret = PS_RET_SDK_UNINIT;
        return ret;
    }

    if (user_) {
        ret = PS_RET_ALREADY_LOGIN;
        log_w("sdk alreay login. ret=%d", PS_RET_ALREADY_LOGIN);
        return ret;
    }

    PushSDKUserInfo* user_ptr = new PushSDKUserInfo;
    *user_ptr                 = user;

    user_.release();
    user_.reset(user_ptr);

    std::shared_ptr<PushRegReq> req = make_login_packet();
    if (!req) {
        ret = PS_RET_ENCODE_LOGIN_PKT_FAILED;
        log_e("encode login packet failed. ret={}",
              PS_RET_ENCODE_LOGIN_PKT_FAILED);
        return PS_RET_ENCODE_LOGIN_PKT_FAILED;
    }

    client_->Send(req);

    return ret;
}

void PushSDK::OnMessage(std::shared_ptr<PushData> msg)
{
    log_w("msg:{}", msg->uri());
}

}  // namespace edu