#pragma once
#include <string>
#define RES_TIMEOUT 100   // SDK认为应答超时
#define RES_SUCCESS 200   /* 功能成功完成,一切正常,不必担心 */
#define RES_ACCEPTED 202  /* 功能请求已接受,等候处理 */
#define RES_NOCHANGED 204 /* 结果没有变，用于CHECKSUM */

#define RES_ERETRY 300 /* 暂时无法受理,建议稍后再试 */

#define RES_EREQUEST 400 /* 语义有误/请求参数有误；服务器无法理解的功能请求 */
#define RES_EAUTH 401 /* 请求者身份未经认证,因此后台拒绝完成功能 */
#define RES_EPERM 403     /* 对方实体没授权(可不是后台不受理) */
#define RES_ENONEXIST 404 /* 目标(对象或用户)不存在 */
#define RES_EACCESS 405   /* 无权限请求此项功能 */
#define RES_EQUOTA 406    /* 用户数据存储超过了限额 */
#define RES_EVOLATILE 407 /* 某些有生存时间限制的资源已经超时不可用 */
#define RES_ETIMEOUT 408  /* 请求过程超时 */
#define RES_ECONFLICT 409 /* 资源或者对象冲突(比如重名) */
#define RES_EPARAM 414 /* 参数数据出错.(越界,超长,不完整等原因) */
#define RES_EDBERROR 415  /* 数据库操作失败 */
#define RES_EDBNVALID 416 /* 数据库暂时不可用，可能是正在维护 */
#define RES_NODEMOVED 418 /* 节点已经被转移*/
#define RES_EOVERTIMES 453 /* 操作次数太多了 */

#define RES_EUNKNOWN 500 /* 出错了.但是原因未明,或是不便透露给你 */
#define RES_EBUSY 504 /* 后台忙,拒绝处理 */
#define RES_EPROT 505 /* 后台不支持此协议版本 */
#define RES_EDATANOSYNC 506 /* 与数据库数据不同步,client需要重新获得数据 */
#define RES_ENOTENOUGH 507     /* 数量不够 */
#define RES_ENONEXIST_REAL 508 /* 目标(对象或用户)不存在 */
#define RES_ESERVICE 550       /* 不支持的服务 */
#define RES_EDAEMON 551        /* 服务器未找到 */
#define RES_EUNUSABLE 552      /* 服务暂时不可用 */
#define RES_ECONNMISS 553      /* 内部错误，根据connid找不到连接 */
#define RES_EBUFOVR 554 /* 发送请求给内部服务器时，缓冲不够，瓶颈保护用。*/
#define RES_ECONNECT 555  /* 内部服务连接不上 */
#define RES_ESENDREQ 556  /* 发送请求发生异常 */
#define RES_EHASHTYPE 557 /* 内部配置错误 */
#define RES_EPACKET 558   /* 请求数据包错误 */
#define RES_ELOCATE 559 /* 定位错误，服务器收到不属于自己的请求 */
#define RES_LIMITUSER 580 /* 不允许一台机器上登录两个相同账号 */
                          //
                          //
                          // typedef unsigned short RES_CODE;
//
// enum class signal_errc {
//  kNull = 0,
//  kSuccess = 200,
//  kNochanged = 204,
//  kRetry = 300,
//  kNonExist = 404,
//  kQuota = 406,
//  kVolatile = 407,
//  kConflict = 409,
//  kDaemon = 551,
//  kHashType = 557,
//};
//
// std::string message(int i) const override {
//  switch (static_cast<signal_errc>(i)) {
//  case signal_errc::kNull: return "empty";
//  case signal_errc::kSuccess: return "成功";
//  case signal_errc::kNochanged: return "结果没有变，用于CHECKSUM";
//  case signal_errc::kRetry: return "暂时无法受理,建议稍后再试";
//  case signal_errc::kNonExist: return "目标(对象或用户)不存在";
//  case signal_errc::kQuota: return "用户数据存储超过了限额";
//  case signal_errc::kVolatile: return
//  "某些有生存时间限制的资源已经超时不可用"; case signal_errc::kConflict:
//  return "资源或者对象冲突(比如重名)"; case signal_errc::kDaemon: return
//  "服务器未找到"; case signal_errc::kHashType: return "内部配置错误";
//  default:return "unknow";
//  }
//}