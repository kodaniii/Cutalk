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
	
	Error_Json = 0x101,		//Json��������
	RPCFailed = 0x102,		//RPC���Ӵ���

	VerifyExpired = 0x103,	//��֤�����
	VerifyCodeErr = 0x104,	//��֤�����
	UserExist = 0x105,		//�û��Ѿ����ڣ��ظ�ע��
	PasswdErr = 0x106,		//ȷ����������벻һ��

	EmailNotRegistered = 0x107,		//������û�б�ע�������������������
	UsernameCannotUse = 0x108,		//���û����������û�ռ�ã������������û���
	ResetUpdateFailed = 0x109,		//�����û���������ʧ��

	LoginFailed = 0x10a,            //��¼���û������������
};

// Defer��
class Defer {
public:
	// ����һ��lambda���ʽ���ߺ���ָ��
	Defer(std::function<void()> func) : _func(func) {}

	// ����������ִ�д���ĺ���
	~Defer() {
		_func();
	}

private:
	std::function<void()> _func;
};