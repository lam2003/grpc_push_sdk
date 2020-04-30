#ifndef PUSH_SDK_PACKET_H
#define PUSH_SDK_PACKET_H

#include <core/client.h>
#include <push_sdk.h>

namespace edu {

extern std::shared_ptr<PushRegReq>
make_login_packet(uint32_t               uid,
                  uint64_t               appid,
                  uint64_t               appkey,
                  const PushSDKUserInfo* user,
                  int64_t                now);

extern std::shared_ptr<PushRegReq>
make_logout_packet(uint32_t uid, uint64_t appid, uint64_t appkey, int64_t now);

extern std::shared_ptr<PushRegReq>
make_join_group_packet(uint32_t uid, uint64_t gtype, uint64_t gid, int64_t now);

extern std::shared_ptr<PushRegReq>
make_join_group_packet(uint32_t                                 uid,
                       const std::multimap<uint64_t, uint64_t>& groups,
                       int64_t                                  now);

extern std::shared_ptr<PushRegReq> make_leave_group_packet(uint32_t uid,
                                                           uint64_t gtype,
                                                           uint64_t gid,
                                                           int64_t  now);

extern UserTerminalType get_user_terminal_type();
}  // namespace edu
#endif