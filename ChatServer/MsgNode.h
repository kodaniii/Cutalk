#pragma once
#include <string>
#include "defs.h"
#include <iostream>
#include <boost/asio.hpp>

using namespace std;
using boost::asio::ip::tcp;
class LogicSystem;

//从TCP缓冲区接收到MsgNode对象，再经本地处理
class MsgNode
{
public:
	MsgNode(short max_len): _total_len(max_len), _cur_len(0) {
		std::cout << "MsgNode::MsgNode()" << endl;
		//_data = new char[_total_len + 1];
		_data = new char[_total_len + 1]();
		_data[_total_len] = '\0';
	}

	~MsgNode() {
		std::cout << "MsgNode::~MsgNode()" << endl;
		delete[] _data;
	}

	void Clear() {
		::memset(_data, 0, _total_len);
		_cur_len = 0;
	}

	short _cur_len;
	short _total_len;
	char* _data;
};

class RecvNode: public MsgNode {
	friend class LogicSystem;
public:
	RecvNode(short max_len, short msg_type_id);
private:
	short _msg_type_id;
};

class SendNode: public MsgNode {
	friend class LogicSystem;
public:
	SendNode(const char *msg, short max_len, short msg_type_id);
private:
	short _msg_type_id;
};

