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

enum HttpReqId{
    REQ_GET_VERIFY_CODE = 0x01, //获取验证码
    REQ_REG_USER = 0x02,        //注册新用户
};

enum Modules{
    MOD_REGISTER = 0x01,
};

enum StatusCodes{
    SUCCESS = 0,
    ERR_JSON = 0x01,            //json解析失败
    ERR_NETWORK = 0x02,         //网络错误
};

extern QString GateServer_url_perfix;

#endif // DEFS_H
