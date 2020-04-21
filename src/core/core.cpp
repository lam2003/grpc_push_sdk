#include <common/log.h>
#include <core/core.h>

#define SDK_LOGGER_NAME "push_sdk"
#define GRPC_LOGGER_NAME "grpc"

namespace edu {

std::shared_ptr<Log> _sdk_logger  = nullptr;
std::shared_ptr<Log> _grpc_logger = nullptr;

static void init_logger()
{
    _sdk_logger = std::make_shared<Log>(SDK_LOGGER_NAME);
    _sdk_logger->LogOnConsole(true);
    _sdk_logger->Initialize();
}

ServiceMeshSDK::ServiceMeshSDK() {}

int ServiceMeshSDK::Initialize()
{
    init_logger();
    log_d("{} {}", "helle", "debug");
    log_e("{} {}", "helle", "error");
    log_i("{} {}", "helle", "info");
    log_w("{} {}", "helle", "warn");
    log_t("{} {}", "helle", "trace");
    log_c("{} {}", "helle", "critical");

    return 0;
}

ServiceMeshSDK::~ServiceMeshSDK() {}

void ServiceMeshSDK::SayHello()
{
    printf("hello \n");
}
}  // namespace edu