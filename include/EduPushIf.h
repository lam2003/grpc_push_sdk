#pragma once
#include <stdint.h>
#include <string>

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef WINDOWS
#define ITRANS_API __declspec(dllexport)
#else
#define ITRANS_API
#endif

    extern "C" ITRANS_API int sayhello();

#ifdef __cplusplus
}
#endif
