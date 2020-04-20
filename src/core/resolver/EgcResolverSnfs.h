#pragma once

#include "EgcResolverConfig.h"
#include "EgcResolverPlugin.h"

namespace grpc_core {

namespace /*EduDns*/ {

class SfnsResolver : public Resolver {
 public:
  explicit SfnsResolver(ResolverArgs args);
  void StartLocked() override;
  void RequestReresolutionLocked() override;
  void ResetBackoffLocked() override;
  void ShutdownLocked() override;

 private:
  virtual ~SfnsResolver();

  void MaybeStartResolvingLocked();
  void StartResolvingLocked();

  static void OnNextResolutionLocked(void* arg, grpc_error* error);
  static void OnResolvedLocked(void* arg, grpc_error* error);

 private:
  /// name to resolve
  char* name_to_resolve_ = nullptr;
  /// channel args
  grpc_channel_args* channel_args_ = nullptr;
  /// pollset_set to drive the name resolution process
  grpc_pollset_set* interested_parties_ = nullptr;
  /// are we shutting down?
  bool shutdown_ = false;
  /// are we currently resolving?
  bool resolving_ = false;
  grpc_closure on_resolved_;
  /// next resolution timer
  bool have_next_resolution_timer_ = false;
  grpc_timer next_resolution_timer_;
  grpc_closure on_next_resolution_;
  /// min time between DNS requests
  grpc_millis min_time_between_resolutions_;
  /// timestamp of last DNS request
  grpc_millis last_resolution_timestamp_ = -1;
  /// retry backoff state
  BackOff backoff_;
  /// currently resolving addresses
  grpc_resolved_addresses* addresses_ = nullptr;
};

class SfnsResolverFactory : public ResolverFactory {
 public:
  OrphanablePtr<Resolver> CreateResolver(ResolverArgs args) const override {
    return OrphanablePtr<Resolver>(New<SfnsResolver>(std::move(args)));
  }

  /// Returns the URI scheme that this factory implements.
  /// Caller does NOT take ownership of result.
  const char* scheme() const override {
#if USE_DNS_SFNS
    return "dns";
#else
    return "edu100dns";
#endif
  }
};

}  // namespace

}  // namespace grpc_core
