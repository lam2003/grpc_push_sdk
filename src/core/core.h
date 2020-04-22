#ifndef EDU_SERVICE_MESH_CORE_H
#define EDU_SERVICE_MESH_CORE_H

#include <common/singleton.h>

namespace edu {

class ServiceMeshSDK : public Singleton<ServiceMeshSDK> {
    friend class Singleton<ServiceMeshSDK>;

  public:
    virtual ~ServiceMeshSDK();
    virtual int Initialize();

  protected:
    ServiceMeshSDK();
};

}  // namespace edu

#endif