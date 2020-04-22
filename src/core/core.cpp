
#include <common/log.h>
#include <core/core.h>

namespace edu {

ServiceMeshSDK::ServiceMeshSDK() {}

int ServiceMeshSDK::Initialize()
{
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