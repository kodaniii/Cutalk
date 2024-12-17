#ifndef DEFS_H
#define DEFS_H

#include <QDebug>
#include <QWidget>
#include <functional>
#include <QRegularExpression>
#include <memory>
#include <iostream>
#include <mutex>
#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkReply>
#include <QDir>
#include <QSettings>
#include "QStyle"

/**
 * @brief qss refresh
**/
//void repolish(QWidget *a);
extern std::function<void(QWidget*)> repolish;

extern std::function<QString(QString)> xorString;

enum ReqId{
    //GateServer
    REQ_GET_VERIFY_CODE = 0x01, //获取验证码
    REQ_REG_USER = 0x02,        //注册新用户
    REQ_RESET_PSWD = 0x03,      //重置密码
    REQ_USER_LOGIN = 0x04,      //用户登录

    //ChatServer
    REQ_CHAT_LOGIN = 0x05,      //登录聊天服务器
    REQ_CHAT_LOGIN_RSP = 0x06,  //登录聊天服务器回包
};

enum Modules{
    MOD_REGISTER = 0x01,        //注册
    MOD_RESET = 0x02,           //密码重置
    MOD_LOGIN = 0x03            //登录
};

enum TipErr{
    TIP_SUCCESS = 0,           //成功
    TIP_EMAIL_ERR = 1,         //邮箱格式错误
    TIP_USER_ERR = 2,          //用户名错误
    TIP_PSWD_ERR = 3,          //密码错误
    TIP_CONFIRM_PSWD_ERR = 4,  //确认密码错误
    TIP_PSWD_CONFIRM_ERR = 5,  //密码和确认密码不匹配
    TIP_VERIFY_CODE_ERR = 6,   //验证码错误

};

enum StatusCodes{
    Success = 0,

    Error_Json = 0x101,         //json解析失败
    GateFailed = 0x102,         //GateServer服务器连接错误
    VerifyFailed = 0x103,       //Verify服务器连接错误
    StatusFailed = 0x104,       //Status服务器连接错误

    VerifyExpired = 0x201,      //验证码过期
    VerifyCodeErr = 0x202,      //验证码错误

    EmailExist = 0x301,         //邮箱已经注册过，重复注册
    UserExist = 0x302,          //用户名已被其他用户占用，但邮箱没有被注册过
    PasswdErr = 0x303,          //确认密码和密码不一致

    EmailNotRegistered = 0x401,		//该邮箱没有被注册过，不允许重置密码
    UsernameCannotUse = 0x402,		//该用户名被其他用户占用，不允许重置用户名
    ResetUpdateFailed = 0x403,		//重置用户名和密码失败


    LoginHandlerFailed = 0x501,     //客户端找不到handler
    LoginFailed = 0x502,            //登录的用户名或密码错误
    TokenInvalid = 0x503,			//Token失效
    UidInvalid = 0x504, 			//uid无效


};

struct ServerInfo{
    QString host;
    QString port;
    QString token;
    int uid;
};

enum LabelClickState{
    Unselected = 0,
    Selected = 1
};

extern QString GateServer_url_perfix;

#endif // DEFS_H
