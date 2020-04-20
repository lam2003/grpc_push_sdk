#pragma once

#if WIN32
#define IRESOLVER_API __declspec(dllexport)
#else
#define ITRANS_API
#endif

namespace EduPcResolver {
  IRESOLVER_API bool RegisterOnce(const char* logpath);
}
