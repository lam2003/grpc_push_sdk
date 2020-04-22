
#include <common/log.h>
#include <core/core.h>
#include <push_sdk.h>

namespace edu {

PushSDK::PushSDK()
{
    appid_   = 0;
    uid_     = 0;
    app_key_ = 0;
    init_    = false;
}

int PushSDK::Initialize()
{
    if (init_) {
        log_w("push_sdk already initialized");
        return PS_RET_ALREADY_INIT;
    }

    log_i("push_sdk initialize successfully");
    init_ = true;
    return 0;
}

PushSDK::~PushSDK() {}
}  // namespace edu