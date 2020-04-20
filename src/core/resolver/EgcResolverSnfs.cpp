#include "EgcResolverSnfs.h"
#include "EgcResolveAddress.h"
#include "common/SLog.h"


#if USE_WIN32_SNFS
extern grpc_address_resolver_vtable* grpc_resolve_address_impl;
static grpc_address_resolver_vtable* default_resolver;
#endif

namespace grpc_core {

#define GRPC_DNS_INITIAL_CONNECT_BACKOFF_SECONDS 1
#define GRPC_DNS_RECONNECT_BACKOFF_MULTIPLIER 1.6
#define GRPC_DNS_RECONNECT_MAX_BACKOFF_SECONDS 120
#define GRPC_DNS_RECONNECT_JITTER 0.2

namespace /*EduDns*/ {

/*explicit*/
SfnsResolver::SfnsResolver(ResolverArgs args)
    : Resolver(args.combiner, std::move(args.result_handler)),
      backoff_(
          BackOff::Options()
              .set_initial_backoff(GRPC_DNS_INITIAL_CONNECT_BACKOFF_SECONDS *
                                   1000)
              .set_multiplier(GRPC_DNS_RECONNECT_BACKOFF_MULTIPLIER)
              .set_jitter(GRPC_DNS_RECONNECT_JITTER)
              .set_max_backoff(GRPC_DNS_RECONNECT_MAX_BACKOFF_SECONDS * 1000)) {
  gpr_log(GPR_INFO, "====create SfnsResolver=====");
  char* path = args.uri->path;
  if (path[0] == '/') ++path;
  name_to_resolve_ = gpr_strdup(path);
  channel_args_ = grpc_channel_args_copy(args.args);
  const grpc_arg* arg = grpc_channel_args_find(
      args.args, GRPC_ARG_DNS_MIN_TIME_BETWEEN_RESOLUTIONS_MS);
  min_time_between_resolutions_ =
      grpc_channel_arg_get_integer(arg, {1000, 0, INT_MAX});
  interested_parties_ = grpc_pollset_set_create();
  if (args.pollset_set != nullptr) {
    grpc_pollset_set_add_pollset_set(interested_parties_, args.pollset_set);
  }
  GRPC_CLOSURE_INIT(&on_next_resolution_, SfnsResolver::OnNextResolutionLocked,
                    this, grpc_combiner_scheduler(args.combiner));
  GRPC_CLOSURE_INIT(&on_resolved_, SfnsResolver::OnResolvedLocked, this,
                    grpc_combiner_scheduler(args.combiner));
  GLOGV("===name to resolver {}", name_to_resolve_);
}

SfnsResolver::~SfnsResolver() {
  grpc_channel_args_destroy(channel_args_);
  grpc_pollset_set_destroy(interested_parties_);
  gpr_free(name_to_resolve_);
}

void SfnsResolver::StartLocked() { MaybeStartResolvingLocked(); }

void SfnsResolver::RequestReresolutionLocked() {
  if (!resolving_) {
    MaybeStartResolvingLocked();
  }
}

void SfnsResolver::ResetBackoffLocked() {
  if (have_next_resolution_timer_) {
    grpc_timer_cancel(&next_resolution_timer_);
  }
  backoff_.Reset();
}

void SfnsResolver::ShutdownLocked() {
  shutdown_ = true;
  if (have_next_resolution_timer_) {
    grpc_timer_cancel(&next_resolution_timer_);
  }
}

void SfnsResolver::OnNextResolutionLocked(void* arg, grpc_error* error) {
  GLOGV("====OnNextResolutionLocked========");
  SfnsResolver* r = static_cast<SfnsResolver*>(arg);
  r->have_next_resolution_timer_ = false;
  if (error == GRPC_ERROR_NONE && !r->resolving_) {
    r->StartResolvingLocked();
  }
  r->Unref(DEBUG_LOCATION, "retry-timer");
}

void SfnsResolver::OnResolvedLocked(void* arg, grpc_error* error) {
  GLOGV("OnResolvedLocked");

  SfnsResolver* r = static_cast<SfnsResolver*>(arg);
  GPR_ASSERT(r->resolving_);
  r->resolving_ = false;
  if (r->shutdown_) {
    r->Unref(DEBUG_LOCATION, "edusfns-pc-resolving");
    return;
  }
  if (r->addresses_ != nullptr) {
    Result result;
    for (size_t i = 0; i < r->addresses_->naddrs; ++i) {
      result.addresses.emplace_back(&r->addresses_->addrs[i].addr,
                                    r->addresses_->addrs[i].len,
                                    nullptr /* args */);
    }
    grpc_resolved_addresses_destroy(r->addresses_);
    result.args = grpc_channel_args_copy(r->channel_args_);
    r->result_handler()->ReturnResult(std::move(result));
    // Reset backoff state so that we start from the beginning when the
    // next request gets triggered.
    r->backoff_.Reset();
  } else {
    gpr_log(GPR_INFO, "edu dns resolution failed (will retry): %s",
            grpc_error_string(error));
    // Return transient error.
    r->result_handler()->ReturnError(grpc_error_set_int(
        GRPC_ERROR_CREATE_REFERENCING_FROM_STATIC_STRING(
            "DNS resolution failed", &error, 1),
        GRPC_ERROR_INT_GRPC_STATUS, GRPC_STATUS_UNAVAILABLE));
    // Set up for retry.
    grpc_millis next_try = r->backoff_.NextAttemptTime();
    grpc_millis timeout = next_try - ExecCtx::Get()->Now();
    GPR_ASSERT(!r->have_next_resolution_timer_);
    r->have_next_resolution_timer_ = true;
    // TODO(roth): We currently deal with this ref manually.  Once the
    // new closure API is done, find a way to track this ref with the timer
    // callback as part of the type system.
    r->Ref(DEBUG_LOCATION, "next_resolution_timer").release();
    if (timeout > 0) {
      gpr_log(GPR_DEBUG, "retrying in %" PRId64 " milliseconds", timeout);
    } else {
      gpr_log(GPR_DEBUG, "retrying immediately");
    }
    grpc_timer_init(&r->next_resolution_timer_, next_try,
                    &r->on_next_resolution_);
  }
  r->Unref(DEBUG_LOCATION, "edudns-resolving");
}

void SfnsResolver::MaybeStartResolvingLocked() {
  gpr_log(GPR_INFO, "====SfnsResolver::MaybeStartResolvingLocked======");
  // If there is an existing timer, the time it fires is the earliest time we
  // can start the next resolution.
  if (have_next_resolution_timer_) return;
  if (last_resolution_timestamp_ >= 0) {
    const grpc_millis earliest_next_resolution =
        last_resolution_timestamp_ + min_time_between_resolutions_;
    const grpc_millis ms_until_next_resolution =
        earliest_next_resolution - grpc_core::ExecCtx::Get()->Now();
    if (ms_until_next_resolution > 0) {
      const grpc_millis last_resolution_ago =
          grpc_core::ExecCtx::Get()->Now() - last_resolution_timestamp_;
      gpr_log(GPR_DEBUG,
              "In cooldown from last resolution (from %" PRId64
              " ms ago). Will resolve again in %" PRId64 " ms",
              last_resolution_ago, ms_until_next_resolution);
      have_next_resolution_timer_ = true;
      // TODO(roth): We currently deal with this ref manually.  Once the
      // new closure API is done, find a way to track this ref with the timer
      // callback as part of the type system.
      Ref(DEBUG_LOCATION, "next_resolution_timer_cooldown").release();
      grpc_timer_init(&next_resolution_timer_, ms_until_next_resolution,
                      &on_next_resolution_);
      return;
    }
  }
  StartResolvingLocked();
}

void SfnsResolver::StartResolvingLocked() {
  gpr_log(GPR_DEBUG, "SfnsResolver Start resolving.");
  // TODO(roth): We currently deal with this ref manually.  Once the
  // new closure API is done, find a way to track this ref with the timer
  // callback as part of the type system.
  Ref(DEBUG_LOCATION, "dns-resolving").release();
  GPR_ASSERT(!resolving_);
  resolving_ = true;
  addresses_ = nullptr;

  grpc_resolve_address(name_to_resolve_, kDefaultPort, interested_parties_,
                       &on_resolved_, &addresses_);
  last_resolution_timestamp_ = grpc_core::ExecCtx::Get()->Now();
}
}  // namespace
}  // namespace grpc_core

static grpc_address_resolver_vtable sfns_resolver = {
    sfns_resolve_address, sfns_blocking_resolve_address};

void grpc_resolver_sfns_init() {
  // grpc_init();
  GLOGV("========grpc_resolver_sfns_init==========");

  std::string resolver_type = "edu100dns";
  grpc_core::UniquePtr<char> resolver =
      GPR_GLOBAL_CONFIG_GET(grpc_dns_resolver);

  if (strlen(resolver.get()) != 0) {
    gpr_log(GPR_INFO, "Warning: overriding resolver setting of %s",
            resolver.get());
  }

  if (gpr_stricmp(resolver_type.c_str(), "edu100dns") == 0) {
#if USE_DNS_SFNS
    GPR_GLOBAL_CONFIG_SET(grpc_dns_resolver, "edu100dns");
#endif
#if USE_NATIVE
    GPR_GLOBAL_CONFIG_SET(grpc_dns_resolver, "native");
#endif

  } else {
    gpr_log(GPR_ERROR, "--resolver_type was not set to edu100dns");
  }

  grpc_core::ResolverRegistry::Builder::InitRegistry();

#if USE_WIN32_SNFS
  if (default_resolver == nullptr) {
    default_resolver = grpc_resolve_address_impl;
  }
  grpc_set_resolver_impl(&sfns_resolver);
#endif

  grpc_core::ResolverRegistry::Builder::RegisterResolverFactory(
      grpc_core::UniquePtr<grpc_core::ResolverFactory>(
          grpc_core::New<grpc_core::SfnsResolverFactory>()));

  // grpc_core::UniquePtr<char> resolver2 =
  //  GPR_GLOBAL_CONFIG_GET(grpc_dns_resolver);
}

void grpc_resolver_sfns_shutdown() {
  GLOGV("===grpc_resolver_sfns_shutdown ===");
  //  grpc_shutdown();
}
