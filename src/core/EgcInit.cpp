#include "EgcInit.h"
#include <sstream>
#include "EgcRpcInc.h"
#include "common/SLog.h"
namespace EgcTrans {
static grpc::GrpcLibraryCodegen* g_libpin = nullptr;

void _log_func(gpr_log_func_args* args) {
  std::stringstream ss;
  ss << args->message << "===[" << args->file << ":" << args->line << "] ";
  GLOGV(ss.str());
  // log(ss.str(), args->severity != GPR_LOG_SEVERITY_ERROR);
  // std::cout << "[GRPC]" << ss.str() << std::endl;
}

void ConfigureGrpc() {
  gpr_set_log_verbosity(GPR_LOG_SEVERITY_DEBUG);
  gpr_log_verbosity_init();

  gpr_set_log_function(&_log_func);

  // list_tracers all
  // grpc_tracer_set_enabled("list_tracers", 1);
  // grpc_tracer_set_enabled("http2_stream_state", 1);
  // grpc_tracer_set_enabled("bdp_estimator", 1);
  // grpc_tracer_set_enabled("cares_resolver", 1);
  // grpc_tracer_set_enabled("cares_address_sorting", 1);
  grpc_tracer_set_enabled("subchannel", 1);
  grpc_tracer_set_enabled("client_channel_routing", 1);
  grpc_tracer_set_enabled("client_channel_call", 1);
  grpc_tracer_set_enabled("connectivity_state", 1);
  grpc_tracer_set_enabled("call_error", 1);
  grpc_tracer_set_enabled("pick_first", 1);
  // grpc_tracer_set_enabled("round_robin", 1);
  grpc_tracer_set_enabled("channel", 1);
  grpc_tracer_set_enabled("op_failure", 1);
  // grpc_tracer_set_enabled("resolver_refcount", 1);
  // grpc_tracer_set_enabled("flowctl", 1);
}
bool EgcInitGrpcLib(std::string log) {
  // Indicates that a logger has been set
  static bool logger_set = false;
  if (logger_set) {
    SLOGE("EgcInitGrpcLib ALREADY INIT ");
    return false;
  }

  EgcCommon::EgcGrpcLog::init(log);
  logger_set = true;
  return true;
}

void PinLibrary() { g_libpin = new grpc::GrpcLibraryCodegen(); }
void UnpinLibrary() {
  if (g_libpin != nullptr) {
    delete g_libpin;
    g_libpin = nullptr;
  }
}
}  // namespace EgcTrans
