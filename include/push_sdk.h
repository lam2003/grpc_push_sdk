#ifndef EDU_PUSH_SDK_SDK_H
#define EDU_PUSH_SDK_SDK_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WINDOWS
#    define PS_EXPORT __declspec(dllexport)
#else
#    define PS_EXPORT
#endif

// Push SDK API返回码
typedef enum {
    PS_RET_SUCCESS               = 0,  // 成功
    PS_RET_ALREADY_INIT          = 1,  // 重复初始化sdk
    PS_RET_INIT_LOG_FAILED       = 2,  // 初始化日志库失败
    PS_RET_CREATE_CHANNEL_FAILED = 3,  // 创建GRPC Channel失败
    PS_RET_CREATE_STREAM_FAILED  = 4,  // 创建GRPC Stream失败
    PS_RET_UNKNOW
} PushSDKRetCode;

// GRPC调用返回码
typedef enum {
    PS_CALL_RES_OK      = 0,  // 成功
    PS_CALL_RES_ILLEGAL = 1,  // 参数非法
    PS_CALL_RES_TIMEOUT = 2,  // 超时
    PS_CALL_RES_UNKNOW
} PushSDKCallRes;

// 网络连接状态
typedef enum {
    PS_CONN_STATE_OK       = 0,  // 连接正常
    PS_CONN_STATE_NO_READY = 1,  // 未就绪
    PS_CONN_STATE_FINISH   = 2,  // 连接已结束
    PS_CONN_STATE_UNKNOW
} PushSDKConnState;

typedef struct
{
    uint64_t gtype;
    uint64_t gid;
} PushSDKGroupInfo;

typedef struct
{
    uint32_t uid;
    uint64_t appid;
    char*    token;
    uint32_t token_len;
    char*    account;
    uint32_t account_len;
    char*    passwd;
    uint32_t passwd_len;
} PushSDKUserInfo;

// @brief     初始化，必须最先调用，线程安全，重复调用返回成功
// @param[in] logdir 日志存放目录
// @reture    SDK API返回码
PS_EXPORT PushSDKRetCode PushSDKInitialize(const char* logdir);

// @brief     去初始化，线程安全，可重复调用
PS_EXPORT void PushSDKDestroy(void);

// /**
// @brief GRPC Call回调
// @param [in] code 错误码
// @param [in] desc 错误描述
// @param [in] data 自定义指针
// */
// typedef void (*PushSDKCallCB)(PushSDKCallRes code,
//                               const char*    desc,
//                               void*          data);

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
