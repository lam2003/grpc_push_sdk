#pragma once
#include <stdint.h>
#include <string>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t egc_uid_t;

#ifdef SMPSDK_EXPORT
#    define ITRANS_API __declspec(dllexport)
#else
#    define ITRANS_API
#endif
enum SmsTransCallErrType {
    CALL_OK      = 0,
    CALL_ILLEGAL = 1,
    CALL_TIMEOUT = 2,
    CALL_ERR     = 3,
};

//返回码
enum SmsTransReturnCode {
    ERR_SMS_SUCCESS  = 0,   //接口调用正常
    ERR_SMS_INIT_LOG = 10,  //日志初始化失败
};

/**
@brief 通用成功回调
@param [in] data 自定义指针
*/
typedef void (*SmsTrans_SucCallback)(void* data);

/**
@brief 通用失败回调
@param [in] code 错误码
@param [in] desc 错误描述
@param [in] data 自定义指针
*/
typedef void (*SmsTrans_ErrCallback)(SmsTransCallErrType code,
                                     const char*         desc,
                                     void*               data);

struct SmsTransUserGroup
{
    uint64_t uid;
    uint64_t userGroupType = 1;
    uint64_t userGroupId   = 2;
};

struct SmsTransUserInfo
{
    egc_uid_t uid;
    uint64_t  appid;
    char*     token;
    uint32_t  token_size;
    char*     account;
    uint32_t  account_size;
    char*     passwd;
    uint32_t  passwd_size;
};

typedef enum {
    E_LINK_CREATE_OK = 1,
    //链路创建成功
    E_LINK_CREATE_RETRY,
    //链路创建失败重试中
    E_LINK_LOGINTING,
    E_LINK_LOGIN_OK,
    E_LINK_LOGIN_FAIL,
    // E_LINK_LOGIN_FAIL =4, 这种应该让回调来返回
    E_LINK_FAILURE,
    E_LINK_SHUTDOWN,
    E_LINK_LOGOUT_OK,
    E_LINK_LOGOUT_FAIL,
} SmsTransLinkStatus;

typedef enum {
    PUSH_MSG_BY_UID = 0,
    PUSH_MSG_BY_GROUP,
} SmsTransPushMsgType;

typedef uint32_t egc_handler_t;
typedef void (*SmsTrans_Linkstatus_Callback)(SmsTransLinkStatus status,
                                             void*              arg);
typedef void (*SmsTrans_Pushmsg_Callback)(const char* data,
                                          uint32_t    size,
                                          void*       arg);
extern "C" egc_handler_t
SmsTrans_AddLinkStatusListener(SmsTrans_Linkstatus_Callback link_cb, void* arg);
extern "C" egc_handler_t
SmsTrans_AddUidMessageListener(std::string               servicename,
                               uint64_t                  uid,
                               SmsTrans_Pushmsg_Callback msg_cb,
                               void*                     arg);
extern "C" egc_handler_t
                SmsTrans_AddGroupMessageListener(std::string               servicename,
                                                 uint64_t                  grouptype,
                                                 uint64_t                  groupid,
                                                 SmsTrans_Pushmsg_Callback msg_cb,
                                                 void*                     arg);
extern "C" bool SmsTrans_RemoveLinkStatusListener(egc_handler_t);
extern "C" bool SmsTrans_RemoveUidMessageListener(egc_handler_t);
extern "C" bool SmsTrans_RemoveGroupMessageListener(egc_handler_t);

extern "C" ITRANS_API SmsTransReturnCode
SmsTrans_RegisterOnce(const char* logpath);

extern "C" ITRANS_API SmsTransReturnCode SmsTrans_Start(egc_uid_t uid,
                                                        uint64_t  appid  = 0,
                                                        uint64_t  appkey = 0);
extern "C" ITRANS_API SmsTransReturnCode
SmsTrans_UserLogin(SmsTransUserInfo     user,
                   SmsTrans_SucCallback suc,
                   SmsTrans_ErrCallback err,
                   void*                data);
extern "C" ITRANS_API SmsTransReturnCode
SmsTrans_UserJoinGroup(SmsTransUserGroup    group,
                       SmsTrans_SucCallback suc,
                       SmsTrans_ErrCallback err,
                       void*                data);
extern "C" ITRANS_API SmsTransReturnCode
SmsTrans_UserLeaveGroup(SmsTransUserGroup    group,
                        SmsTrans_SucCallback suc,
                        SmsTrans_ErrCallback err,
                        void*                data);
extern "C" ITRANS_API SmsTransReturnCode
SmsTrans_UserLogout(SmsTransUserInfo     user,
                    SmsTrans_SucCallback suc,
                    SmsTrans_ErrCallback err,
                    void*                data);

#ifdef __cplusplus
}
#endif
