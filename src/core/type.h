#ifndef EDU_PUSH_SDK_TYPE_H
#define EDU_PUSH_SDK_TYPE_H

#include <proto/pushGateWay.grpc.pb.h>

using PushGateway        = grpc::push::gateway::PushGateway;
using Stub               = grpc::push::gateway::PushGateway::Stub;
using PushRegReq         = grpc::push::gateway::PushRegReq;
using LoginRequest       = grpc::push::gateway::LoginRequest;
using LoginResponse      = grpc::push::gateway::LoginResponse;
using LogoutRequest      = grpc::push::gateway::LogoutRequest;
using LogoutResponse     = grpc::push::gateway::LogoutResponse;
using JoinGroupRequest   = grpc::push::gateway::JoinGroupRequest;
using JoinGroupResponse  = grpc::push::gateway::JoinGroupResponse;
using LeaveGroupRequest  = grpc::push::gateway::LeaveGroupRequest;
using LeaveGroupResponse = grpc::push::gateway::LeaveGroupResponse;
using UserGroup          = grpc::push::gateway::UserGroup;
using PushData           = grpc::push::gateway::PushData;
using StreamURI          = grpc::push::gateway::StreamURI;
using UserTerminalType   = grpc::push::gateway::UserTerminalType;
using RW =
    grpc::ClientAsyncReaderWriterInterface<grpc::push::gateway::PushRegReq,
                                           grpc::push::gateway::PushData>;

namespace edu {

enum class ClientEvent {
    CONNECTED  = 1,
    READ_DONE  = 2,
    WRITE_DONE = 3,
    FINISHED   = 4
};

enum class StreamStatus {
    WAIT_CONNECT    = 100,
    CONNECTED       = 101,
    READY_TO_WRITE  = 102,
    WAIT_WRITE_DONE = 103,
    FINISHED        = 104
};

enum class ChannelState { OK, NO_READY };

extern std::string channel_state_to_string(ChannelState state);
extern std::string client_status_to_string(StreamStatus status);
extern std::string stream_uri_to_string(StreamURI uri);

}  // namespace edu

#endif