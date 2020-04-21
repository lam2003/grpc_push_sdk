#ifndef EDU_SERVICE_MESH
#define EDU_SERVICE_MESH

#include <common/singleton.h>

namespace edu {
class Config : public Singleton<Config> {
    friend Singleton<Config>;

  public:
    virtual ~Config();

  protected:
    Config();

  public:
    // bool        EnableGrpcLogOnConsole = true;
    // std::string GrpcLogDir             = "./grpc_log";
    // std::string GrpcLogLevel           = "debug";
    
}
}  // namespace edu

#endif