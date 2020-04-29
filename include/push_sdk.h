#ifndef EDU_PUSH_SDK_SDK_H
#define EDU_PUSH_SDK_SDK_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#    define PS_EXPORT __declspec(dllexport)
#else
#    define PS_EXPORT
#endif

// Push SDK API返回码
typedef enum {
    PS_RET_SUCCESS            = 0,  // 成功
    PS_RET_ALREADY_INIT       = 1,  // 重复初始化SDK
    PS_RET_INIT_LOG_FAILED    = 2,  // 初始化日志库失败
    PS_RET_SDK_UNINIT         = 3,  // SDK未初始化
    PS_RET_ALREADY_LOGIN      = 4,  // SDK已经登录，请先登出
    PS_RET_REQ_ENC_FAILED     = 5,  // 请求包序列化失败
    PS_RET_USER_INFO_IS_NULL  = 6,  // 登录传入的用户信息为空
    PS_RET_CB_IS_NULL         = 7,  // 回调函数为空
    PS_RET_ALREADY_JOIN_GROUP = 8,  // 该组已经加入
    PS_RET_UNLOGIN            = 9,  // 未登录
    PS_RET_UNKNOW
} PushSDKRetCode;

// Push SDK回调类型
typedef enum {
    PS_CB_TYPE_LOGIN       = 0,  // 登录回调
    PS_CB_TYPE_LOGOUT      = 1,  // 登出回调
    PS_CB_TYPE_JOIN_GROUP  = 2,  // 加组回调
    PS_CB_TYPE_LEAVE_GROUP = 3,  // 离组回调
    PS_CB_TYPE_INNER_ERR   = 4   // 内部错误回调
} PushSDKCBType;

// 回调事件返回码
typedef enum {
    PS_CB_EVENT_OK                 = 0,  // 成功
    PS_CB_EVENT_FAILED             = 1,  // 失败
    PS_CB_EVENT_TIMEOUT            = 2,  // 调用超时
    PS_CB_EVENT_REQ_ENC_FAILED     = 3,  // 序列化请求包失败
    PS_CB_EVENT_DEC_FAILED         = 4,  // 去序列化回复包失败
    PS_CB_EVENT_USER_KICKED_BY_SRV = 5   // 登录后被踢下线

} PushSDKCBEvent;

// 网络连接状态
typedef enum {
    PS_CONN_STATE_OK       = 0,  // 连接正常
    PS_CONN_STATE_NO_READY = 1,  // 连接未就绪
    PS_CONN_STATE_UNKNOW   = 2
} PushSDKConnState;

typedef struct
{
    char token[256];
    char account[256];
    char passwd[256];
} PushSDKUserInfo;

typedef struct
{
    uint64_t gtype;
    uint64_t gid;
} PushSDKGroupInfo;

/**
@brief SDK回调
@param [in] code 错误码
@param [in] desc 错误描述
@param [in] data 自定义指针
*/
typedef void (*PushSDKCallCB)(PushSDKCBType  type,
                              PushSDKCBEvent res,
                              const char*    desc,
                              void*          data);

// @brief     初始化，必须最先调用，线程安全，重复调用返 PS_RET_ALREADY_INIT
// @param[in] uid 用户ID，SDK初始化时用于路由
// @param[in] appid 应用ID
// @param[in] appkey 应用密钥
// @param[in] logdir 日志存放目录
// @param[in] cb_func 全局事件回调函数
// @param[in] cb_arg 全局事件回调函数args
// @reture    SDK API返回码
PS_EXPORT PushSDKRetCode PushSDKInitialize(uint32_t      uid,
                                           uint64_t      appid,
                                           uint64_t      appkey,
                                           const char*   logdir,
                                           PushSDKCallCB cb_func,
                                           void*         cb_arg);

// @brief     去初始化，线程安全，可重复调用
PS_EXPORT void PushSDKDestroy(void);

// @brief     异步登录，线程安全，必须在SDK初始化之后调用，重复调用返回
// PS_RET_ALREADY_LOGIN
// @param[in] user 用户信息
// @param[in] cb_func 回调函数
// @param[in] cb_arg 回调函数args
// @return    SDK API返回码
PS_EXPORT PushSDKRetCode PushSDKLogin(PushSDKUserInfo* user,
                                      PushSDKCallCB    cb_func,
                                      void*            cb_arg);

// @brief     同步登录，线程安全，必须在SDK初始化之后调用，重复调用返回
// PS_RET_ALREADY_LOGIN
// @param[in] user 用户信息
// @return    SDK API返回码
PS_EXPORT PushSDKRetCode PushSDKLoginSync(PushSDKUserInfo* user);

// @brief     异步登出，线程安全，必须在SDK初始化之后调用，重复调用返回成功
// @param[in] cb_func 回调函数
// @param[in] cb_arg 回调函数args
// @return    SDK API返回码
PS_EXPORT PushSDKRetCode PushSDKLogout(PushSDKCallCB cb_func, void* cb_arg);

// @brief     同步登出，线程安全，必须在SDK初始化之后调用，重复调用返回成功
// @return    SDK API返回码
PS_EXPORT PushSDKRetCode PushSDKLogoutSync();

// @brief
// 异步进组，线程安全，必须在SDK初始化之后调用，重复进组返回PS_RET_ALREADY_JOIN_GROUP
// @param[in] group 组信息
// @param[in] cb_func 回调函数
// @param[in] cb_arg 回调函数args
// @return    SDK API返回码
PS_EXPORT PushSDKRetCode PushSDKJoinGroup(PushSDKGroupInfo* group,
                                          PushSDKCallCB     cb_func,
                                          void*             cb_arg);

// @brief
// 同步进组，线程安全，必须在SDK初始化之后调用，重复进组返回PS_RET_ALREADY_JOIN_GROUP
// @param[in] group 组信息
// @return    SDK API返回码
PS_EXPORT PushSDKRetCode PushSDKJoinGroupSync(PushSDKGroupInfo* group);

// @brief
// 异步离组，线程安全，必须在SDK初始化之后调用，重复离组返回成功
// @param[in] group 组信息
// @param[in] cb_func 回调函数
// @param[in] cb_arg 回调函数args
// @return    SDK API返回码
PS_EXPORT PushSDKRetCode PushSDKLeaveGroup(PushSDKGroupInfo* group,
                                           PushSDKCallCB     cb_func,
                                           void*             cb_arg);

// @brief
// 同步离组，线程安全，必须在SDK初始化之后调用，重复进组返回PS_RET_ALREADY_JOIN_GROUP
// @param[in] group 组信息
// @return    SDK API返回码
PS_EXPORT PushSDKRetCode PushSDKLeaveGroupSync(PushSDKGroupInfo* group);

// typedef void* ps_hdl_t;
// typedef void (*SmsTransLinkstatus_Callback)(PushSDKConnState state, void*
// arg); typedef void (*SmsTransPushmsg_Callback)(const char* data,
//                                          uint32_t    size,
//                                          void*       arg);
// extern "C" ps_hdl_t
// SmsTransAddLinkStatusListener(SmsTransLinkstatus_Callback link_cb, void*
// arg); extern "C" ps_hdl_t SmsTransAddUidMessageListener(const char*
// servicename,
//                               uint64_t                 uid,
//                               SmsTransPushmsg_Callback msg_cb,
//                               void*                    arg);
// extern "C" ps_hdl_t
//                 SmsTransAddGroupMessageListener(const char* servicename,
//                                                 uint64_t grouptype, uint64_t
//                                                 groupid,
//                                                 SmsTransPushmsg_Callback
//                                                 msg_cb, void* arg);
// extern "C" bool SmsTransRemoveLinkStatusListener(ps_hdl_t);
// extern "C" bool SmsTransRemoveUidMessageListener(ps_hdl_t);
// extern "C" bool SmsTransRemoveGroupMessageListener(ps_hdl_t);

// extern "C" PS_EXPORT PushSDKRetCode SmsTransStart(uint32_t uid,
//                                                   uint64_t appid  = 0,
//                                                   uint64_t appkey = 0);
// extern "C" PS_EXPORT PushSDKRetCode SmsTransUserLogin(PushSDKUserInfo user,
//                                                       PushSDKCallCB   cb,
//                                                       void*           data);
// extern "C" PS_EXPORT PushSDKRetCode
// SmsTransUserJoinGroup(PushSDKGroupInfo group, PushSDKCallCB cb, void* data);
// extern "C" PS_EXPORT PushSDKRetCode
//                                     SmsTransUserLeaveGroup(PushSDKGroupInfo
//                                     group, PushSDKCallCB cb, void* data);
// extern "C" PS_EXPORT PushSDKRetCode SmsTransUserLogout(PushSDKUserInfo user,
//                                                        PushSDKCallCB   cb,
//                                                        void*           data);

#ifdef __cplusplus
}
#endif

#endif
