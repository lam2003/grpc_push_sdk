#pragma once

#include "EgcResolverConfig.h"


extern void (*sfns_resolve_address)(const char* name, const char* default_port,
                                    grpc_pollset_set* interested_parties,
                                    grpc_closure* on_done,
                                    grpc_resolved_addresses** addrs);

extern grpc_error* (*sfns_blocking_resolve_address)(
    const char* name, const char* default_port,
    grpc_resolved_addresses** addresses);
