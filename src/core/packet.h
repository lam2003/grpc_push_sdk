#ifndef PUSH_SDK_PACKET_H
#define PUSH_SDK_PACKET_H

#include <core/client.h>
#include <push_sdk.h>

namespace edu {

extern std::shared_ptr<PushRegReq> make_login_packet(uint32_t         uid,
                                                     uint64_t         appid,
                                                     uint64_t         appkey,
                                                     PushSDKUserInfo* user,
                                                     int64_t          now);

extern std::shared_ptr<PushRegReq>
make_logout_packet(uint32_t uid, uint64_t appid, uint64_t appkey, int64_t now);

extern std::shared_ptr<PushRegReq>
make_join_group_packet(uint32_t uid, uint64_t gtype, uint64_t gid, int64_t now);
}  // namespace edu
#endif