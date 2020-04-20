#include <cstring>
#include <conio.h>

#include "EgcResolveAddress.h"
#include "EgdGslbSdk.h"
#include "SLog.h"
// windows平台
#ifdef GRPC_WINSOCK_SOCKET

typedef struct {
  char* name;
  char* default_port;
  grpc_closure request_closure;
  grpc_closure* on_done;
  grpc_resolved_addresses** addresses;
} request;


grpc_error* ResolveByGetaddrinfo(const char* host, char* port, grpc_resolved_addresses** addresses)
{
  std::cout << "==============ResolveByGetaddrinfo host " << host << " port " << port << std::endl;
  size_t i;
  grpc_error* error = GRPC_ERROR_NONE;
  int s;
  struct addrinfo hints;
  struct addrinfo *result = NULL, *resp;

  /* Call getaddrinfo */
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC; /* ipv4 or ipv6 */
  hints.ai_socktype = SOCK_STREAM; /* stream socket */
  hints.ai_flags = AI_PASSIVE; /* for wildcard IP address */

  GRPC_SCHEDULING_START_BLOCKING_REGION;
  s = getaddrinfo(host, port, &hints, &result);
  GRPC_SCHEDULING_END_BLOCKING_REGION;
  if (s != 0)
  {
    error = GRPC_WSA_ERROR(WSAGetLastError(), "getaddrinfo");
    goto done;
  }

  /* Success path: set addrs non-NULL, fill it in */
  (*addresses) =
    (grpc_resolved_addresses*)gpr_malloc(sizeof(grpc_resolved_addresses));
  (*addresses)->naddrs = 0;
  for (resp = result; resp != NULL; resp = resp->ai_next)
  {
    (*addresses)->naddrs++;
  }
  (*addresses)->addrs = (grpc_resolved_address*)gpr_malloc(
    sizeof(grpc_resolved_address) * (*addresses)->naddrs);
  i = 0;
  for (resp = result; resp != NULL; resp = resp->ai_next)
  {
    memcpy(&(*addresses)->addrs[i].addr, resp->ai_addr, resp->ai_addrlen);
    (*addresses)->addrs[i].len = resp->ai_addrlen;

    ///////////BEGIN PRINT 打印解析结果
    char* address_uri = grpc_sockaddr_to_uri(&(*addresses)->addrs[i]);
    if (address_uri)
    {
      std::cout << "Snfs==>addr:=== begin" << address_uri << std::endl;
      std::cout << "Snfs==>addr:========end" << std::endl;
      gpr_free(address_uri);
    }
    else
    {
      std::cout << "===>>error !!! not got addr !!!" << std::endl;
    }
    /////////END PRINT
    i++;
  }

  {
    for (i = 0; i < (*addresses)->naddrs; i++)
    {
      char* buf;
      grpc_sockaddr_to_string(&buf, &(*addresses)->addrs[i], 0);
      gpr_free(buf);
    }
  }
  //getaddrinfo失败
done:
  if (result)
  {
    freeaddrinfo(result);
  }
  return error;
}

bool isValidIpAddress(char* st)
{
  int num, i, len;
  char* ch;

  //counting number of quads present in a given IP address
  int quadsCnt = 0;

  //printf("Split IP: \"%s\"\n", st);

  len = strlen(st);

  //  Check if the string is valid
  if (len < 7 || len > 15)
    return false;
  char* pTmp = nullptr;
  ch = strtok_s(st, ".", &pTmp);

  while (ch != NULL)
  {
    quadsCnt++;
    printf("Quald %d is %s\n", quadsCnt, ch);

    num = 0;
    i = 0;

    //  Get the current token and convert to an integer value
    while (ch[i] != '\0')
    {
      num = num * 10;
      num = num + (ch[i] - '0');
      i++;
    }

    if (num < 0 || num > 255)
    {
      //printf("Not a valid ip\n");
      return false;
    }

    if ((quadsCnt == 1 && num == 0) || (quadsCnt == 4 && num == 0))
    {
      //printf("Not a valid ip, quad: %d AND/OR quad:%d is zero\n", quadsCnt, quadsCnt);
      return false;
    }

    ch = strtok_s(NULL, ".",&pTmp);
  }

  //  Check the address string, should be n.n.n.n format
  if (quadsCnt != 4)
  {
    return false;
  }

  //  Looks like a valid IP address
  return true;
}

grpc_error* ResolveByGslb(const char* host, char* port, grpc_resolved_addresses** addresses)
{
  std::cout << "==============ResolveByGslb host " << host  <<" port "<<port << std::endl;
  size_t i;
  int j ;
  grpc_error* error = GRPC_ERROR_NONE;

  GRPC_SCHEDULING_START_BLOCKING_REGION;
  EduGslb::DomainIps ips = EduGslb::GetIpByDomain(host);
  GRPC_SCHEDULING_END_BLOCKING_REGION;
  if (ips.success == false)
  {
    std::string err_resason = "EduGslb::GetIpByDomain" + std::string(host);
    error = GRPC_WSA_ERROR(WSAGetLastError(), err_resason.c_str());
    GLOGV("err : glsb fail {} {}", err_resason, ips.description);
    std::cout << " resolve by gslb fail " << ips.description << std::endl;
    goto done;
  }

  /* Success path: set addrs non-NULL, fill it in */
  (*addresses) =
    (grpc_resolved_addresses*)gpr_malloc(sizeof(grpc_resolved_addresses));
  (*addresses)->naddrs = 0;
  //for (resp = result; resp != NULL; resp = resp->ai_next)
  //{
  //  (*addresses)->naddrs++;
  //}
  (*addresses)->naddrs = ips.ipcount;
  (*addresses)->addrs = (grpc_resolved_address*)gpr_malloc(
    sizeof(grpc_resolved_address) * (*addresses)->naddrs);

  for (j=0; j < ips.ipcount;j++)
  {
    std::string ipstr = std::string(ips.ips[j]);
    uint32_t port_unit32 = std::stoi(std::string(port));
    GLOGV("===>glsbresult ipaddr:j {}  {} port_unit32 {}",j, ipstr, port_unit32);
    grpc_string_to_sockaddr(&(*addresses)->addrs[j], ips.ips[j], port_unit32);
    ///////////BEGIN PRINT 打印解析结果
    char* address_uri = grpc_sockaddr_to_uri(&(*addresses)->addrs[j]);

    if (address_uri)
    {
      std::cout << "Snfs==>addr:="<<j<<"== begin" << address_uri << std::endl;
      std::cout << "Snfs==>addr:========end" << std::endl;
      gpr_free(address_uri);
    }
    else
    {
      std::cout << "===>>error EduGslb::GetIpByDomain not got addr !!!" << std::endl;
    }
    /////////END PRINT
  }

  {
    for (i = 0; i < (*addresses)->naddrs; i++)
    {
      char* buf;
      grpc_sockaddr_to_string(&buf, &(*addresses)->addrs[i], 0);
      gpr_free(buf);
    }
  }
  //getaddrinfo失败
done:
  return error;
}

static grpc_error* sfns_blocking_resolve_address_impl(const char* name, const char* default_port,
                                                      grpc_resolved_addresses** addresses)
{
  gpr_log(GPR_DEBUG, "===>sfns_blocking_resolve_address_impl ");
  std::cout << "=========sfns_blocking_resolve_address_impl============" << std::endl;
  std::string hoststr;
  grpc_core::ExecCtx exec_ctx;
  char* host;
  char* port;
  grpc_error* error = GRPC_ERROR_NONE;
  /* parse name, splitting it into host and port parts */
  gpr_split_host_port(name, &host, &port);
  if (host == NULL)
  {
    char* msg;
    gpr_asprintf(&msg, "unparseable host:port: '%s'", name);
    error = GRPC_ERROR_CREATE_FROM_COPIED_STRING(msg);
    gpr_free(msg);
    goto done;
  }
  if (port == NULL)
  {
    if (default_port == NULL)
    {
      char* msg;
      gpr_asprintf(&msg, "no port in name '%s'", name);
      error = GRPC_ERROR_CREATE_FROM_COPIED_STRING(msg);
      gpr_free(msg);
      goto done;
    }
    port = gpr_strdup(default_port);
  }
  
#if  1
  hoststr = std::string(host);
  if(isValidIpAddress(host))
  {
    std::cout << "host is valid ip so gslb will not work ,ResolveByGetaddrinfo" << host << std::endl;
    goto getaddr;
  }
  if(EduGslb::IsSdkReady())
  {
    error = ResolveByGslb(hoststr.c_str(), port, addresses);
    if (error != GRPC_ERROR_NONE)
    {
      std::cout << "err to ResolveByGslb ,retry ResolveByGetaddrinfo host " << host << " port " << port << std::endl;
    }
    else
    {
      goto done;
    }
  }else
  {
    std::cout << " EduGslb is not ready ,so ResolveByGetaddrinfo " << std::endl;
  }
  
#endif
getaddr:
  error = ResolveByGetaddrinfo(hoststr.c_str(), port, addresses);
  if (error == GRPC_ERROR_NONE)
  {
    std::cout << " ResolveByGetaddrinfo ok host " << host << " port " << port << std::endl;
  }else
  {
    std::cout << " ResolveByGetaddrinfo fail host " << host << " port " << port << std::endl;
  }


done:
  gpr_free(host);
  gpr_free(port);

  return error;
}

/* Callback to be passed to grpc_executor to asynch-ify
 * grpc_blocking_resolve_address */
static void do_request_thread(void* rp, grpc_error* error)
{
  request* r = (request*)rp;
  if (error == GRPC_ERROR_NONE)
  {
    error =
      grpc_blocking_resolve_address(r->name, r->default_port, r->addresses);
  }
  else
  {
    GRPC_ERROR_REF(error);
  }
  GRPC_CLOSURE_SCHED(r->on_done, error);
  gpr_free(r->name);
  gpr_free(r->default_port);
  gpr_free(r);
}

static void sfns_resolve_address_impl(const char* name,
                                      const char* default_port,
                                      grpc_pollset_set* interested_parties,
                                      grpc_closure* on_done,
                                      grpc_resolved_addresses** addresses)
{
  gpr_log(GPR_INFO, "===>sfns_resolve_address_impl ");
  std::cout << "=========sfns_resolve_address_impl============" << std::endl;
  request* r = (request*)gpr_malloc(sizeof(request));
  GRPC_CLOSURE_INIT(
    &r->request_closure, do_request_thread, r,
    grpc_core::Executor::Scheduler(grpc_core::ExecutorType::RESOLVER,
      grpc_core::ExecutorJobType::SHORT));
  r->name = gpr_strdup(name);
  r->default_port = gpr_strdup(default_port);
  r->on_done = on_done;
  r->addresses = addresses;
  GRPC_CLOSURE_SCHED(&r->request_closure, GRPC_ERROR_NONE);
}

// grpc_address_resolver_vtable grpc_windows_resolver_vtable = {
//	windows_resolve_address, sfns_windows_blocking_resolve_address };
//

//指针赋值
void (*sfns_resolve_address)(
  const char* name, const char* default_port,
  grpc_pollset_set* interested_parties, grpc_closure* on_done,
  grpc_resolved_addresses** addrs) = sfns_resolve_address_impl;

grpc_error* (*sfns_blocking_resolve_address)(
  const char* name, const char* default_port,
  grpc_resolved_addresses** addresses) = sfns_blocking_resolve_address_impl;

#endif
