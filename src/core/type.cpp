#include <core/type.h>

namespace edu {

std::string channel_state_to_string(ChannelState state)
{
    switch (state) {
        case ChannelState::OK: {
            return "OK";
        }
        case ChannelState::NO_READY: {
            return "NO_READY";
        }
        default: {
            return "UNKNOW";
        }
    }
}

std::string client_status_to_string(ClientStatus status)
{
    switch (status) {
        case ClientStatus::FINISHED: {
            return "FINISHED";
        }
        case ClientStatus::READY_TO_WRITE: {
            return "READY_TO_WRITE";
        }
        case ClientStatus::WAIT_CONNECT: {
            return "WAIT_CONNECT";
        }
        case ClientStatus::WAIT_WRITE_DONE: {
            return "WAIT_WRITE_DONE";
        }
        case ClientStatus::CONNECTED: {
            return "CONNECTED";
        }
        default: {
            return "UNKNOW";
        }
    }
}

std::string stream_uri_to_string(StreamURI uri)
{
    switch (uri) {
        case StreamURI::PPushGateWayLoginURI: {
            return "PPushGateWayLoginURI";
        }
        case StreamURI::PPushGateWayLoginResURI: {
            return "PPushGateWayLoginResURI";
        }
        case StreamURI::PPushGateWayLogoutURI: {
            return "PPushGateWayLogoutURI";
        }

        case StreamURI::PPushGateWayLogoutResURI: {
            return "PPushGateWayLogoutResURI";
        }
        case StreamURI::PPushGateWayJoinGroupURI: {
            return "PPushGateWayJoinGroupURI";
        }
        case StreamURI::PPushGateWayJoinGroupResURI: {
            return "PPushGateWayJoinGroupResURI";
        }
        case StreamURI::PPushGateWayLeaveGroupURI: {
            return "PPushGateWayLeaveGroupURI";
        }
        case StreamURI::PPushGateWayLeaveGroupResURI: {
            return "PPushGateWayLeaveGroupResURI";
        }
        case StreamURI::PPushGateWayPingURI: {
            return "PPushGateWayPingURI";
        }
        case StreamURI::PPushGateWayPongURI: {
            return "PPushGateWayPongURI";
        }
        case StreamURI::PPushGateWayNotifyToCloseURI: {
            return "PPushGateWayNotifyToCloseURI";
        }
        case StreamURI::PPushGateWayPushDataByUidURI: {
            return "PPushGateWayPushDataByUidURI";
        }
        case StreamURI::PPushGateWayPushDataByGroupURI: {
            return "PPushGateWayPushDataByGroupURI";
        }
        case StreamURI::PPushGateWayUNKNOWN:
        default: {
            return "UNKNOW";
        }
    }
}

}  // namespace edu