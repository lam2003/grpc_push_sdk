#include "common/PlatformDefine.h"
namespace EgcCommon {
TerminalType getTerminalType() {
  TerminalType type;
#ifdef _WIN32
  // define something for Windows (32-bit and 64-bit, this part is common
#ifdef _WIN64
  // define something for Windows (64-bit only)
  type = E_TT_WINDOWS_64;
#else
  // define something for Windows (32-bit only)
  type = E_TT_WINDOWS_32;
#endif

#elif __APPLE__
#include "TargetConditionals.h"

#if TARGET_IPHONE_SIMULATOR
  // iOS Simulator
  type = E_TT_IOS_SIMULATOR;
#elif TARGET_OS_IPHONE
  // iOS device
  type = E_TT_IOS;
#elif TARGET_OS_MAC
  // Other kinds of Mac OS
  type = E_TT_MAC;
#else
#error "Unknown Apple platform"
  type = E_TT_APPLE_UNKNOWN;
#endif
#elif __ANDROID__
  // android
  type = E_TT_ANDROID;
#elif __linux__
  type = E_TT_LINUX;
  // linux
#elif __unix__  // all unices not caught above
  type = E_TT_UNIX;
  // Unix
#elif defined(_POSIX_VERSION)
  // POSIX
  type = E_TT_POSIX_UNKNOWN;
#else
#error "Unknown compiler"
  type = E_TT_UNKNOWN;
#endif
  return type;
}
}  // namespace EgcCommon
