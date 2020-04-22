#ifndef EDU_PUSH_SDK_SDK_H
#define EDU_PUSH_SDK_SDK_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WINDOWS
#    define ITRANS_API __declspec(dllexport)
#else
#    define ITRANS_API
#endif

// Push SDK API返回码
typedef enum {
    PS_RET_SUCCESS      = 0,  // 成功
    PS_RET_ALREADY_INIT = 1,  // 重复初始化sdk
    PS_RET_INIT_LOG     = 2,  // 初始化日志库失败
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
} PushSDKUserGroupInfo;

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

/**
@brief GRPC Call回调
@param [in] code 错误码
@param [in] desc 错误描述
@param [in] data 自定义指针
*/
typedef void (*PushSDKCallCB)(PushSDKCallRes code,
                              const char*    desc,
                              void*          data);

typedef uint32_t egc_handler_t;
typedef void (*SmsTransLinkstatus_Callback)(PushSDKConnState state, void* arg);
typedef void (*SmsTransPushmsg_Callback)(const char* data,
                                         uint32_t    size,
                                         void*       arg);
extern "C" egc_handler_t
SmsTransAddLinkStatusListener(SmsTransLinkstatus_Callback link_cb, void* arg);
extern "C" egc_handler_t
SmsTransAddUidMessageListener(const char*              servicename,
                              uint64_t                 uid,
                              SmsTransPushmsg_Callback msg_cb,
                              void*                    arg);
extern "C" egc_handler_t
                SmsTransAddGroupMessageListener(const char*              servicename,
                                                uint64_t                 grouptype,
                                                uint64_t                 groupid,
                                                SmsTransPushmsg_Callback msg_cb,
                                                void*                    arg);
extern "C" bool SmsTransRemoveLinkStatusListener(egc_handler_t);
extern "C" bool SmsTransRemoveUidMessageListener(egc_handler_t);
extern "C" bool SmsTransRemoveGroupMessageListener(egc_handler_t);

extern "C" ITRANS_API PushSDKRetCode SmsTransRegisterOnce(const char* logpath);

extern "C" ITRANS_API PushSDKRetCode SmsTransStart(uint32_t uid,
                                                   uint64_t appid  = 0,
                                                   uint64_t appkey = 0);
extern "C" ITRANS_API PushSDKRetCode SmsTransUserLogin(PushSDKUserInfo user,
                                                       PushSDKCallCB    cb,
                                                       void*            data);
extern "C" ITRANS_API PushSDKRetCode
SmsTransUserJoinGroup(PushSDKUserGroupInfo group, PushSDKCallCB cb, void* data);
extern "C" ITRANS_API PushSDKRetCode
                                     SmsTransUserLeaveGroup(PushSDKUserGroupInfo group,
                                                            PushSDKCallCB        cb,
                                                            void*                data);
extern "C" ITRANS_API PushSDKRetCode SmsTransUserLogout(PushSDKUserInfo user,
                                                        PushSDKCallCB    cb,
                                                        void*            data);

#ifdef __cplusplus
}
#endif

#endif
