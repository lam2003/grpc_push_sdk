#pragma once

#include "EgcResolver.h"
#include "EgcResolverInc.h"
#include "EgcResolverPlugin.h"
#include <EgdGslbSdk.h>

namespace EduPcResolver {
  bool RegisterOnce(const char * logpath)
  {
    //resolver ²å¼þ
    grpc_register_plugin(grpc_resolver_sfns_init,
      grpc_resolver_sfns_shutdown);
   return EduGslb::Init(logpath);
  }
}
