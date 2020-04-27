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
    PS_RET_SUCCESS           = 0,  // 成功
    PS_RET_ALREADY_INIT      = 1,  // 重复初始化SDK
    PS_RET_INIT_LOG_FAILED   = 2,  // 初始化日志库失败
    PS_RET_SDK_UNINIT        = 3,  // SDK未初始化
    PS_RET_ALREADY_LOGIN     = 4,  // SDK已经登录，请先登出
    PS_RET_REQ_ENC_FAILED    = 5,  // 请求包序列化失败
    PS_RET_USER_INFO_IS_NULL = 6,  // 登录传入的用户信息为空
    PS_RET_CB_IS_NULL        = 7,  // 回调函数为空
    PS_RET_UNKNOW
} PushSDKRetCode;

// Push SDK回调类型
typedef enum {
    PS_CB_LOGIN     = 0,  // 登录回调
    PS_CB_RELOGIN   = 1,  // SDK内部重新登录回调
    PS_CB_LOGOUT    = 2,  // 登出回调
    PS_CB_INNER_ERR = 3,  // 内部错误回调
} PushSDKCBType;

// GRPC调用返回码
typedef enum {
    PS_CALL_RES_OK         = 0,  // 成功
    PS_CALL_RES_FAILE      = 1,  // 失败
    PS_CALL_TIMEOUT        = 2,  // 调用超时
    PS_CALL_REQ_ENC_FAILED = 3,  // 序列化请求包失败
    PS_CALL_RES_DEC_FAILED = 4   // 去序列化回复包失败

} PushSDKCallRes;

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
                              PushSDKCallRes res,
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

// @brief     登录，线程安全，必须在SDK初始化之后调用，重复调用返回
// PS_RET_ALREADY_INIT
// @param[in] user 用户信息
// @param[in] cb_func 回调函数
// @param[in] cb_arg 回调函数args
// @return    SDK API返回码
PS_EXPORT PushSDKRetCode PushSDKLogin(PushSDKUserInfo* user,
                                      PushSDKCallCB    cb_func,
                                      void*            cb_arg);

// @brief     登出，线程安全，必须在SDK初始化之后调用，重复调用返回成功
// @param[in] cb_func 回调函数
// @param[in] cb_arg 回调函数args
// @return    SDK API返回码
PS_EXPORT PushSDKRetCode PushSDKLogout(PushSDKCallCB cb_func, void* cb_arg);

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
