
#include <common/log.h>
#include <core/client.h>
#include <core/core.h>
#include <push_sdk.h>

namespace edu {

PushSDK::PushSDK()
{
    client_ = std::make_shared<Client>();
    init_   = false;
}

int PushSDK::Initialize()
{
    int ret = PS_RET_SUCCESS;

    if (init_) {
        ret = PS_RET_ALREADY_INIT;
        log_w("push_sdk already initialized");
        return ret;
    }

    if ((ret = client_->Initialize()) != PS_RET_SUCCESS) {
        log_e("client create channel failed. ret=%d", PS_RET_SUCCESS);
        return ret;
    }

    log_i("push_sdk initialize successfully");
    init_ = true;
    return ret;
}

void PushSDK::Destroy()
{
    client_->Destroy();
}

PushSDK::~PushSDK()
{
    Destroy();
}
}  // namespace edu