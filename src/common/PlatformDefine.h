#ifndef PLATFORM_DEFINE_H__
#define PLATFORM_DEFINE_H__

//#include "Cpp11Support.h"

namespace EgcCommon {
enum TerminalType {
  E_TT_UNKNOWN = 0,
  E_TT_WINDOWS_32 = 1,
  E_TT_WINDOWS_64 = 2,
  E_TT_IOS = 3,  //
  E_TT_ANDROID = 4,
  E_TT_IOS_SIMULATOR = 5,
  E_TT_LINUX = 6,
  E_TT_UNIX = 7,
  E_TT_MAC = 8,
  E_TT_APPLE_UNKNOWN = 9,
  E_TT_POSIX_UNKNOWN = 10,
};
TerminalType getTerminalType();
}  // namespace EgcCommon

#endif
