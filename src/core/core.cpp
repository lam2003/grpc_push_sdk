
#include <common/log.h>
#include <core/core.h>

namespace edu {

ServiceMeshSDK::ServiceMeshSDK() {}

int ServiceMeshSDK::Initialize()
{
    log_i("sms initialized");
    return 0;
}

ServiceMeshSDK::~ServiceMeshSDK() {}
}  // namespace edu