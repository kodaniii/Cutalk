#pragma once

#include <boost/beast/http.hpp>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <memory>

#include <iostream>
#include <functional>
#include <map>
#include <unordered_map>

#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>

#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>

#include "hiredis.h"
#include <cassert>

#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/exception.h>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

namespace filesys = boost::filesystem;

#define TCP_BUFFER	8192		//8KB
#define TIME_OUT	30			//30s

#define CODE_PREFIX "code_"

enum ErrorCodes {
    Success = 0,
    UnKnownFailed = 0x100,

    Error_Json = 0x101,         //json解析失败
    GateFailed = 0x102,         //GateServer服务器连接错误
    VerifyFailed = 0x103,       //Verify服务器连接错误
    StatusFailed = 0x104,       //Status服务器连接错误
    ChatFailed = 0x105,         //ChatServer连接错误

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

enum ReqId {
	REQ_CHAT_LOGIN = 0x05,      //登录聊天服务器
	REQ_CHAT_LOGIN_RSP = 0x06,  //登录聊天服务器回包

    REQ_SEARCH_USER = 0x07,     //搜索用户
    REQ_SEARCH_USER_RSP = 0x08, //搜索用户回包

    REQ_ADD_FRIEND_REQ = 0x09,   //添加好友申请
    REQ_ADD_FRIEND_RSP = 0x0a,   //申请添加好友回复
    REQ_NOTIFY_ADD_FRIEND_REQ = 0x0b,  //通知用户添加好友申请
    REQ_AUTH_FRIEND_REQ = 0x0c,  //认证好友请求
    REQ_AUTH_FRIEND_RSP = 0x0d,  //认证好友回复
    REQ_NOTIFY_AUTH_FRIEND_REQ = 0x0e, //通知用户认证好友申请

    REQ_TEXT_CHAT_MSG_REQ = 0x0f,  //文本聊天信息请求
    REQ_TEXT_CHAT_MSG_RSP = 0x10,  //文本聊天信息回复
    REQ_NOTIFY_UPDATE_CHAT_MSG_REQ = 0x11, //通知用户更新聊天文本信息
};

// Defer类
class Defer {
public:
	// 接受一个lambda表达式或者函数指针
	Defer(std::function<void()> func) : _func(func) {}

	// 析构函数中执行传入的函数
	~Defer() {
		_func();
	}

private:
	std::function<void()> _func;
};

//msg_type_id最大长度
#define MAX_LENGTH ((1) << (11))

//头部总长度
#define HEAD_TOTAL_LEN 4
//头部id长度
#define HEAD_ID_LEN 2
//头部数据长度
#define HEAD_DATA_LEN 2

#define MAX_RECVQUE  10000
#define MAX_SENDQUE 1000


#define USER_TOKEN_PREFIX  "u_token_uid_"
#define LOGIN_COUNT  "login_count"
#define USER_IP_PREFIX  "u_ip_uid_"
#define USER_BASE_INFO "u_base_info_uid_"
#define NAME_INFO  "u_base_info_name_"