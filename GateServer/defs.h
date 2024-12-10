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
	Success = 0x00,
	
	Error_Json = 0x101,		//Json解析错误
	RPCFailed = 0x102,		//RPC连接错误

	VerifyExpired = 0x103,	//验证码过期
	VerifyCodeErr = 0x104,	//验证码错误
	UserExist = 0x105,		//用户已经存在，重复注册
	PasswdErr = 0x106,		//确认密码和密码不一致

	EmailNotRegistered = 0x107,		//该邮箱没有被注册过，不允许重置密码
	UsernameCannotUse = 0x108,		//该用户名被其他用户占用，不允许重置用户名
	ResetUpdateFailed = 0x109,		//重置用户名和密码失败

	LoginFailed = 0x10a,            //登录的用户名或密码错误
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