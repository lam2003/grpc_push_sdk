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
    PS_RET_SUCCESS            = 0,   // 成功
    PS_RET_ALREADY_INIT       = 1,   // 重复初始化SDK
    PS_RET_INIT_LOG_FAILED    = 2,   // 初始化日志库失败
    PS_RET_SDK_UNINIT         = 3,   // SDK未初始化
    PS_RET_ALREADY_LOGIN      = 4,   // SDK已经登录，请先登出
    PS_RET_REQ_ENC_FAILED     = 5,   // 请求包序列化失败
    PS_RET_USER_INFO_IS_NULL  = 6,   // 登录传入的用户信息为空
    PS_RET_CB_IS_NULL         = 7,   // 回调函数为空
    PS_RET_ALREADY_JOIN_GROUP = 8,   // 该组已经加入
    PS_RET_UNLOGIN            = 9,   // 未登录
    PS_RET_CALL_TIMEOUT       = 10,  // 调用超时
    PS_RET_CALL_FAILED        = 11   // 服务器返回失败
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
    PS_CB_EVENT_RES_DEC_FAILED     = 4,  // 去序列化回复包失败
    PS_CB_EVENT_USER_KICKED_BY_SRV = 5   // 登录后被踢下线

} PushSDKCBEvent;

// 网络连接状态
typedef enum {
    PS_CONN_STATE_OK       = 0,  // 连接正常
    PS_CONN_STATE_NO_READY = 1   // 连接未就绪
} PushSDKConnState;

typedef struct
{
    char token[512];
    int  token_size;
    char account[512];
    int  account_size;
    char passwd[512];
    int  passwd_size;
} PushSDKUserInfo;

typedef struct
{
    uint64_t gtype;
    uint64_t gid;
} PushSDKGroupInfo;

/**
@brief SDK全局事件回调函数
@param [in] type 事件类型
@param [in] res  回调事件返回码，除了PS_CB_EVENT_TIMEOUT(超时) 和
PS_CB_EVENT_USER_KICKED_BY_SRV(被挤下线)外都需要处理
@param [in] desc 服务器返回的错误描述
@param [in] data 自定义指针
*/
typedef void (*PushSDKEventCB)(PushSDKCBType  type,
                               PushSDKCBEvent res,
                               const char*    desc,
                               void*          data);

typedef void* PS_HANDLER;

/**
@brief SDK用户消息回调
@param [in] data 消息数据
@param [in] len 消息长度
*/
typedef void (*PushSDKUserMsgCB)(const char* data, int len);

/**
@brief SDK组消息回调
@param [in] from_gtype 消息来源组类型
@param [in] frome_gid 消息来源组ID
@param [in] data 消息数据
@param [in] len 消息长度
*/
typedef void (*PushSDKGroupMsgCB)(uint64_t    from_gtype,
                                  uint64_t    from_gid,
                                  const char* data,
                                  int         len);

/**
@brief SDK连接状态回调
@param [in] state 连接状态
*/
typedef void (*PushSDKConnStateCB)(PushSDKConnState state);

// @brief     初始化，必须最先调用，重复调用返 PS_RET_ALREADY_INIT，线程安全
// @param[in] uid 用户ID，SDK初始化时用于路由
// @param[in] appid 应用ID
// @param[in] appkey 应用密钥
// @param[in] logdir 日志存放目录
// @param[in] cb_func 全局事件回调函数
// @param[in] cb_arg 全局事件回调函数args
// @reture    SDK API返回码
PS_EXPORT PushSDKRetCode PushSDKInitialize(uint32_t       uid,
                                           uint64_t       appid,
                                           uint64_t       appkey,
                                           const char*    logdir,
                                           PushSDKEventCB cb_func,
                                           void*          cb_arg);

// @brief     去初始化，可重复调用，线程安全
PS_EXPORT void PushSDKDestroy(void);

// @brief     同步登录，必须在SDK初始化之后调用，重复调用返回，线程安全
// PS_RET_ALREADY_LOGIN
// @param[in] user 用户信息
// @return    SDK API返回码
PS_EXPORT PushSDKRetCode PushSDKLogin(PushSDKUserInfo* user);

// @brief     同步登出，必须在SDK初始化之后调用，重复调用返回成功，线程安全
// @return    SDK API返回码
PS_EXPORT PushSDKRetCode PushSDKLogout();

// @brief
// 同步进组，必须在SDK初始化之后调用，重复进组返回PS_RET_ALREADY_JOIN_GROUP，线程安全
// @param[in] group 组信息
// @return    SDK API返回码
PS_EXPORT PushSDKRetCode PushSDKJoinGroup(PushSDKGroupInfo* group);

// @brief
// 同步离组，必须在SDK初始化之后调用，重复离组返回成功，线程安全
// @param[in] group 组信息
// @return    SDK API返回码
PS_EXPORT PushSDKRetCode PushSDKLeaveGroup(PushSDKGroupInfo* group);

// @brief
// 同步调用返回PS_RET_CALL_FAILED的时候，立刻调用此函数可获得错误描述及返回码，线程安全
// @param[out] desc 服务器返回的错误描述，使用后需要手动free
// @param[out] code 服务器返回的错误码
PS_EXPORT void PushSDKGetError(char** desc, int* code);

// @brief
// 生成句柄，线程安全
// @return 句柄，用于添加回调函数
PS_EXPORT PS_HANDLER PushSDKCreateHandler();

// @brief
// 摧毁句柄，线程安全
// @param[in] handle 句柄
PS_EXPORT void PushSDKDestroyHandler(PS_HANDLER handler);

// @brief
// 添加连接状态回调
// @param[in] handler 句柄
// @param[in] state_cb 连接状态回调
PS_EXPORT void PushSDKSetConnStateCB(PS_HANDLER         handler,
                                     PushSDKConnStateCB state_cb);

// @brief
// 添加用户消息回调
// @param[in] handler 句柄
// @param[in] msg_cb 用户消息回调
PS_EXPORT void PushSDKSetUserMsgCB(PS_HANDLER handler, PushSDKUserMsgCB msg_cb);

// @brief
// 添加组消息回调
// @param[in] handler 句柄
// @param[in] msg_cb 组消息回调
PS_EXPORT void PushSDKSetGroupMsgCB(PS_HANDLER        handler,
                                    PushSDKGroupMsgCB msg_cb);

#ifdef __cplusplus
}
#endif

#endif
