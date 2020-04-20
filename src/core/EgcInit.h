#pragma once
#include <string>
#include "EgcRpcInc.h"

#include <grpcpp/impl/grpc_library.h>

static grpc::internal::GrpcLibraryInitializer g_gli_initializer;

namespace EgcTrans {
bool EgcInitGrpcLib(std::string log);
void ConfigureGrpc();
void PinLibrary();
void UnpinLibrary();

}  // namespace EgcTrans
