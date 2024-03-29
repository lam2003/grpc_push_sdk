#include <common/utils.h>
#include <core/packet.h>

namespace edu {

UserTerminalType get_user_terminal_type()
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

std::shared_ptr<PushRegReq> make_login_packet(uint32_t               uid,
                                              uint64_t               appid,
                                              uint64_t               appkey,
                                              const PushSDKUserInfo* user,
                                              int64_t                now)
{
    LoginRequest login_req;
    login_req.set_uid(uid);
    login_req.set_suid(Utils::GetSUID(uid, get_user_terminal_type()));
    login_req.set_appid(std::to_string(appid));
    login_req.set_appkey(appkey);
    login_req.set_termnialtype(get_user_terminal_type());
    login_req.set_account(std::string(user->account, user->account_size));
    login_req.set_password(std::string(user->passwd, user->passwd_size));
    login_req.set_cookie(std::string(user->token, user->token_size));
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

std::shared_ptr<PushRegReq>
make_logout_packet(uint32_t uid, uint64_t appid, uint64_t appkey, int64_t now)
{
    LogoutRequest logout_req;
    logout_req.set_appid(std::to_string(appid));
    logout_req.set_uid(uid);
    logout_req.set_suid(Utils::GetSUID(uid, get_user_terminal_type()));
    logout_req.set_context(std::to_string(now));
    logout_req.set_appkey(appkey);
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

std::shared_ptr<PushRegReq>
make_join_group_packet(uint32_t uid, uint64_t gtype, uint64_t gid, int64_t now)
{
    JoinGroupRequest jg_req;
    jg_req.set_uid(uid);
    jg_req.set_suid(Utils::GetSUID(uid, get_user_terminal_type()));
    jg_req.set_context(std::to_string(now));

    UserGroup* usergroup = jg_req.add_usergroupset();
    usergroup->set_usergrouptype(gtype);
    usergroup->set_usergroupid(gid);

    std::string msg_data;
    if (!jg_req.SerializeToString(&msg_data)) {
        log_e("JoinGroup packet serialize failed");
        return nullptr;
    }

    std::shared_ptr<PushRegReq> req = std::make_shared<PushRegReq>();
    req->set_uri(StreamURI::PPushGateWayJoinGroupURI);
    req->set_msgdata(msg_data);

    return req;
}

std::shared_ptr<PushRegReq>
make_join_group_packet(uint32_t                                 uid,
                       const std::multimap<uint64_t, uint64_t>& groups,
                       int64_t                                  now)
{
    JoinGroupRequest jg_req;
    jg_req.set_uid(uid);
    jg_req.set_suid(Utils::GetSUID(uid, get_user_terminal_type()));
    jg_req.set_context(std::to_string(now));

    auto it = groups.begin();
    for (; it != groups.end(); it++) {
        UserGroup* usergroup = jg_req.add_usergroupset();
        usergroup->set_usergrouptype(it->first);
        usergroup->set_usergroupid(it->second);
    }

    std::string msg_data;
    if (!jg_req.SerializeToString(&msg_data)) {
        log_e("JoinGroup packet serialize failed");
        return nullptr;
    }

    std::shared_ptr<PushRegReq> req = std::make_shared<PushRegReq>();
    req->set_uri(StreamURI::PPushGateWayJoinGroupURI);
    req->set_msgdata(msg_data);

    return req;
}

std::shared_ptr<PushRegReq>
make_leave_group_packet(uint32_t uid, uint64_t gtype, uint64_t gid, int64_t now)
{
    LeaveGroupRequest lg_req;
    lg_req.set_context(std::to_string(now));
    lg_req.set_suid(Utils::GetSUID(uid, get_user_terminal_type()));
    lg_req.set_uid(uid);
    UserGroup* usergroup = lg_req.add_usergroupset();
    usergroup->set_usergrouptype(gtype);
    usergroup->set_usergroupid(gid);

    std::string msg_data;
    if (!lg_req.SerializeToString(&msg_data)) {
        log_e("LeaveGroup packet serialize failed");
        return nullptr;
    }

    std::shared_ptr<PushRegReq> req = std::make_shared<PushRegReq>();
    req->set_uri(StreamURI::PPushGateWayLeaveGroupURI);
    req->set_msgdata(msg_data);

    return req;
}
}  // namespace edu