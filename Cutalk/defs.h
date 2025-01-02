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

enum class ChatRole
{
    Self,
    Other
};

//聊天界面模式
enum ChatUIMode{
    SearchMode,     //搜索模式
    ChatMode,       //聊天模式
    ContactMode,    //联系人模式
};

struct MsgInfo{
    //"text，image，file"
    QString msgFlag;
    //文本信息，表示文件和图像的url
    QString content;
    //文件和图片的缩略图
    QPixmap pixmap;
};

//自定义QListWidgetItem的几种类型
enum ListItemType{
    CHAT_USER_ITEM,         //好友聊天项，聊天窗口->好友聊天列表
    CONTACT_USER_ITEM,      //联系人用户项，联系人窗口->联系人
    SEARCH_USER_ITEM,       //搜索到的用户
    ADD_USER_TIP_ITEM,      //提示添加用户
    INVALID_ITEM,           //不可点击条目
    GROUP_TIP_ITEM,         //分组标题项，grouptipitem
    LINE_ITEM,              //分割线
    APPLY_FRIEND_ITEM,      //好友申请项，位于好友申请分组下的项
};

enum ReqId{
    //GateServer
    REQ_GET_VERIFY_CODE = 0x01, //获取验证码
    REQ_REG_USER = 0x02,        //注册新用户
    REQ_RESET_PSWD = 0x03,      //重置密码
    REQ_USER_LOGIN = 0x04,      //用户登录

    //ChatServer
    /*所有的REQ请求（有两个没写REQ，懒得改了）都是由TCPMgr sig_tcp_send_data信号发送的
      所有的RSP请求都是由TCPMgr inithandlers()中注册的函数处理的*/
    REQ_CHAT_LOGIN = 0x05,      //登录聊天服务器
    REQ_CHAT_LOGIN_RSP = 0x06,  //登录聊天服务器回包

    REQ_SEARCH_USER = 0x07,     //搜索用户
    REQ_SEARCH_USER_RSP = 0x08, //搜索用户回包

    //我方主动发好友请求
    REQ_ADD_FRIEND_REQ = 0x09,   //添加好友申请
    REQ_ADD_FRIEND_RSP = 0x0a,   //申请添加好友回复，接收是否成功发送到ChatServer的消息
    //对方发好友请求的通知
    REQ_NOTIFY_ADD_FRIEND_REQ = 0x0b,  //通知用户添加好友申请

    //我方同意对方的好友请求
    REQ_AUTH_FRIEND_REQ = 0x0c,  //认证好友请求
    REQ_AUTH_FRIEND_RSP = 0x0d,  //认证好友回复
    REQ_NOTIFY_AUTH_FRIEND_REQ = 0x0e, //通知用户认证好友申请

    REQ_TEXT_CHAT_MSG_REQ  = 0x0f,  //文本聊天信息请求
    REQ_TEXT_CHAT_MSG_RSP  = 0x10,  //文本聊天信息回复
    REQ_NOTIFY_UPDATE_CHAT_MSG_REQ = 0x11, //通知用户更新聊天文本信息
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
    ChatFailed = 0x105,         //ChatServer连接错误
    MysqlFailed = 0x106,
    RedisFailed = 0x107,

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

//申请好友标签输入框最低长度
const int MIN_APPLY_LABEL_ED_LEN = 40;
const int tip_offset = 5;

const QString add_prefix = "添加标签 ";

const int CHAT_COUNT_PER_PAGE = 13;

extern const std::vector<QString> strs;
extern const std::vector<QString> heads;
extern const std::vector<QString> names;
extern const std::vector<QString> apply_friend_strs;



#endif // DEFS_H
