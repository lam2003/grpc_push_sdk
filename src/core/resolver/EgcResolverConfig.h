#pragma once

#include "EgcResolverInc.h"

#if WIN32
//Ŀǰ���õ�resolverʵ��ֻ֧��windows
#define USE_WIN32_SNFS 1

//�����ã��벻Ҫ��
#define USE_NATIVE 0

//dnsģʽ������
#define USE_DNS_SFNS 1

#endif

const char kDefaultPort[] = "https";