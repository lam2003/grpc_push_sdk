#pragma once

#include "EgcResolverInc.h"

#if WIN32
//目前内置的resolver实现只支持windows
#define USE_WIN32_SNFS 1

//测试用，请不要打开
#define USE_NATIVE 0

//dns模式的名字
#define USE_DNS_SFNS 1

#endif

const char kDefaultPort[] = "https";