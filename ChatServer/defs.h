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

    Error_Json = 0x101,         //json����ʧ��
    GateFailed = 0x102,         //GateServer���������Ӵ���
    VerifyFailed = 0x103,       //Verify���������Ӵ���
    StatusFailed = 0x104,       //Status���������Ӵ���
    ChatFailed = 0x105,         //ChatServer���Ӵ���

    VerifyExpired = 0x201,      //��֤�����
    VerifyCodeErr = 0x202,      //��֤�����

    EmailExist = 0x301,         //�����Ѿ�ע������ظ�ע��
    UserExist = 0x302,          //�û����ѱ������û�ռ�ã�������û�б�ע���
    PasswdErr = 0x303,          //ȷ����������벻һ��

    EmailNotRegistered = 0x401,		//������û�б�ע�������������������
    UsernameCannotUse = 0x402,		//���û����������û�ռ�ã������������û���
    ResetUpdateFailed = 0x403,		//�����û���������ʧ��

    LoginHandlerFailed = 0x501,     //�ͻ����Ҳ���handler
    LoginFailed = 0x502,            //��¼���û������������
    TokenInvalid = 0x503,			//TokenʧЧ
    UidInvalid = 0x504, 			//uid��Ч
};

enum ReqId {
	REQ_CHAT_LOGIN = 0x05,      //��¼���������
	REQ_CHAT_LOGIN_RSP = 0x06,  //��¼����������ذ�
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

//msg_type_id��󳤶�
#define MAX_LENGTH ((1) << (11))

//ͷ���ܳ���
#define HEAD_TOTAL_LEN 4
//ͷ��id����
#define HEAD_ID_LEN 2
//ͷ�����ݳ���
#define HEAD_DATA_LEN 2

#define MAX_RECVQUE  10000
#define MAX_SENDQUE 1000


#define USER_TOKEN_PREFIX  "u_token_uid"
#define LOGIN_COUNT  "login_count"
#define USER_IP_PREFIX  "u_ip"
#define USER_BASE_INFO "u_base_info"
#define NAME_INFO  "name_info_"