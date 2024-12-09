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

enum HttpReqId{
    REQ_GET_VERIFY_CODE = 0x01, //获取验证码
    REQ_REG_USER = 0x02,        //注册新用户
    REQ_RESET_PSWD = 0X03       //重置密码
};

enum Modules{
    MOD_REGISTER = 0x01,        //注册
    MOD_RESET = 0x02            //密码重置
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
    SUCCESS = 0,
    ERR_JSON = 0x101,           //json解析失败
    RPCFailed = 0x102,          //RPC连接错误

    VerifyExpired = 0x103,      //验证码过期
    VerifyCodeErr = 0x104,      //验证码错误
    UserExist = 0x105,          //用户已经存在，重复注册
    PasswdErr = 0x106,          //确认密码和密码不一致

    EmailNotRegistered = 0x107,		//该邮箱没有被注册过，不允许重置密码
    UsernameCannotUse = 0x108,		//该用户名被其他用户占用，不允许重置用户名
    ResetUpdateFailed = 0x109		//重置用户名和密码失败

};

enum LabelClickState{
    Unselected = 0,
    Selected = 1
};

extern QString GateServer_url_perfix;

#endif // DEFS_H
