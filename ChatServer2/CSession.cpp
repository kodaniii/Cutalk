#include "CSession.h"
#include "CServer.h"
#include <iostream>
#include <sstream>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include "LogicSystem.h"
#include <string>
#include "RedisMgr.h"
#include "ConfigMgr.h"

using namespace std;

CSession::CSession(boost::asio::io_context& io_context, CServer* server)
	: _socket(io_context), 
	_server(server), 
	_isStop(false), 
	_b_head_parse(false), 
	_user_uid(0){
	cout << "CSession::CSession()" << endl;

	//boost雪花算法
	boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
	_session_id = boost::uuids::to_string(a_uuid);
	cout << "random_generator() _session_id " << _session_id << endl;

	//生成一个头部Msg
	_recv_head_node = make_shared<MsgNode>(HEAD_TOTAL_LEN);
}

CSession::~CSession() {
	std::cout << "~CSession destruct" << endl;

	/* 获取Redis的连接数，并-1 */
	// 这里其实并不是原子性执行的，正常来说应该是HGet、-1、HSet写在同一个函数里，
	// 然后在RedisMgr函数内部加锁
	// TODO 看看后续改不改(已经改完了)
	auto& gCfgMgr = ConfigMgr::init();
	auto server_name = gCfgMgr["SelfServer"]["name"];

	bool isSucc = RedisMgr::GetInstance()->HDec(LOGIN_COUNT, server_name);
	std::cout << "RedisMgr::GetInstance()->HDec() " << (isSucc? "success": "fail") << std::endl;

	return;
}

tcp::socket& CSession::GetSocket() {
	return _socket;
}

std::string& CSession::GetSessionId() {
	return _session_id;
}

void CSession::SetUserId(int uid)
{
	_user_uid = uid;
}

int CSession::GetUserId()
{
	return _user_uid;
}

void CSession::Start(){
	AsyncReadHead(HEAD_TOTAL_LEN);
}

void CSession::Send(std::string msg, short msgid) {
	std::cout << "CSession::Send()" << std::endl;
	std::lock_guard<std::mutex> lk(_send_lock);
	int send_que_size = _send_que.size();
	if (send_que_size > MAX_SENDQUE) {
		std::cout << "session: " << _session_id << ", send que fulled, size = " << MAX_SENDQUE << endl;
		return;
	}

	_send_que.push(make_shared<SendNode>(msg.c_str(), msg.length(), msgid));
	if (send_que_size > 0) {
		return;
	}
	auto& msgnode = _send_que.front();
	boost::asio::async_write(_socket, boost::asio::buffer(msgnode->_data, msgnode->_total_len),
		std::bind(&CSession::HandleWrite, this, std::placeholders::_1, SharedSelf()));
}

void CSession::Send(char* msg, short max_length, short msgid) {
	std::cout << "CSession::Send()" << std::endl;
	std::lock_guard<std::mutex> lk(_send_lock);
	int send_que_size = _send_que.size();
	if (send_que_size > MAX_SENDQUE) {
		std::cout << "session: " << _session_id << ", send que fulled, size = " << MAX_SENDQUE << endl;
		return;
	}

	_send_que.push(make_shared<SendNode>(msg, max_length, msgid));
	if (send_que_size > 0) {
		return;
	}
	auto& msgnode = _send_que.front();
	boost::asio::async_write(_socket, boost::asio::buffer(msgnode->_data, msgnode->_total_len), 
		std::bind(&CSession::HandleWrite, this, std::placeholders::_1, SharedSelf()));
}

void CSession::Close() {
	_socket.close();
	_isStop = true;
}

std::shared_ptr<CSession> CSession::SharedSelf() {
	return shared_from_this();
}

//读取TCP缓冲区头部到this->_recv_head_node
//将头部数据保存至msg_type_id和msg_len，并将大端网络字节序化为本地字节序
//将msg_type_id和msg_len保存至this->_recv_msg_node，然后接收消息体body
void CSession::AsyncReadHead(int total_len)
{
	cout << "CSession::AsyncReadHead()" << endl;
	auto self = shared_from_this();
	//asyncReadFull(size_t maxLen, std::function<...> handler);
	asyncReadFull(HEAD_TOTAL_LEN, 
		[self, this](const boost::system::error_code& ec, std::size_t bytes_transfered) {
		try {
			if (ec) {
				if (ec != boost::asio::error::eof) {
					std::cout << "handle read failed, error " << ec.what() << endl;
				}
				Close();
				_server->ClearSession(_session_id);
				return;
			}

			if (bytes_transfered < HEAD_TOTAL_LEN) {
				std::cout << "read length not match, read \"" << bytes_transfered << "\", total_len = "
					<< HEAD_TOTAL_LEN << endl;
				Close();
				_server->ClearSession(_session_id);
				return;
			}

			//接收到的头部数据this->_data拷贝到_recv_head_node->_data
			_recv_head_node->Clear();
			memcpy(_recv_head_node->_data, _data, bytes_transfered);

			//从_recv_head_node->_data获取头部msg_type_id
			short msg_type_id = 0;
			memcpy(&msg_type_id, _recv_head_node->_data, HEAD_ID_LEN);

			//网络字节序（大端）转化为本地字节序
			msg_type_id = boost::asio::detail::socket_ops::network_to_host_short(msg_type_id);
			std::cout << "msg_type_id = " << msg_type_id << endl;
			//id非法
			if (msg_type_id > MAX_LENGTH) {
				std::cout << "invalid msg_type_id = " << msg_type_id << endl;
				_server->ClearSession(_session_id);
				return;
			}

			short msg_len = 0;
			memcpy(&msg_len, _recv_head_node->_data + HEAD_ID_LEN, HEAD_DATA_LEN);
			//网络字节序转化为本地字节序
			msg_len = boost::asio::detail::socket_ops::network_to_host_short(msg_len);
			std::cout << "msg_len = " << msg_len << endl;

			//id非法
			if (msg_len > MAX_LENGTH) {
				std::cout << "invalid msg_len = " << msg_len << endl;
				_server->ClearSession(_session_id);
				return;
			}

			//将前4字节保存至_recv_msg_node
			_recv_msg_node = make_shared<RecvNode>(msg_len, msg_type_id);
			AsyncReadBody(msg_len);
		}
		catch (std::exception& e) {
			std::cout << "Exception catch " << e.what() << endl;
		}
	});
}

void CSession::AsyncReadBody(int total_len)
{
	cout << "CSession::AsyncReadBody()" << endl;
	auto self = shared_from_this();
	asyncReadFull(total_len, [self, this, total_len](const boost::system::error_code& ec, std::size_t bytes_transfered) {
		try {
			if (ec) {
				if (ec != boost::asio::error::eof) {
					std::cout << "handle read failed, error " << ec.what() << endl;
				}
				Close();
				_server->ClearSession(_session_id);
				return;
			}

			if (bytes_transfered < total_len) {
				std::cout << "read length not match, read \"" << bytes_transfered << "\", total_len = "
					<< total_len << endl;
				Close();
				_server->ClearSession(_session_id);
				return;
			}

			//将从TCP缓冲区接收到的数据_data存储至_recv_msg_node，_recv_msg_node被替换为消息体
			memcpy(_recv_msg_node->_data, _data, bytes_transfered);
			_recv_msg_node->_cur_len += bytes_transfered;
			_recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';
			cout << "receive data body " << _recv_msg_node->_data << endl;
			//此处将消息投递到逻辑队列中
			LogicSystem::GetInstance()->PostMsgToQue(make_shared<LogicNode>(shared_from_this(), _recv_msg_node));
			//继续监听头部接受事件
			AsyncReadHead(HEAD_TOTAL_LEN);
		}
		catch (std::exception& e) {
			std::cout << "Exception catch " << e.what() << endl;
		}
	});
}

void CSession::HandleWrite(const boost::system::error_code& error, std::shared_ptr<CSession> shared_self) {
	std::cout << "CSession::HandleWrite()" << std::endl;
	//增加异常处理
	try {
		if (!error) {
			std::lock_guard<std::mutex> lk(_send_lock);
			//cout << "send data " << _send_que.front()->_data + HEAD_LENGTH << endl;
			_send_que.pop();
			//非空继续处理
			if (!_send_que.empty()) {
				auto& msgnode = _send_que.front();
				boost::asio::async_write(_socket, boost::asio::buffer(msgnode->_data, msgnode->_total_len),
					std::bind(&CSession::HandleWrite, this, std::placeholders::_1, shared_self));
			}
		}
		else {
			if (error != boost::asio::error::eof) {
				std::cout << "handle write failed, error is " << error.what() << endl;
			}
			Close();
			_server->ClearSession(_session_id);
		}
	}
	catch (std::exception& e) {
		std::cerr << "Exception catch " << e.what() << endl;
	}
}

//读取完整长度
void CSession::asyncReadFull(std::size_t maxLength, std::function<void(const boost::system::error_code&, std::size_t)> handler) {
	::memset(_data, 0, MAX_LENGTH);
	asyncReadLen(0, maxLength, handler);
}

//读取指定字节数，使用async_read_some封装
//async_read_some支持读取指定长度，并不完全读完
//read_len表示读取首地址（相当于已经读取的长度），total_len表示总读取长度（包含已经读取的长度）
//出错或顺利结束，回调函数返回ec和已读长度
void CSession::asyncReadLen(std::size_t read_len, std::size_t total_len, 
	std::function<void(const boost::system::error_code&, std::size_t)> handler) {
	auto self = shared_from_this();
	_socket.async_read_some(boost::asio::buffer(_data + read_len, total_len - read_len),
		[read_len, total_len, handler, self](const boost::system::error_code& ec, std::size_t bytesTransfered) {
			if (ec) {
				// 出现错误，调用回调函数
				handler(ec, read_len + bytesTransfered);
				return;
			}

			if (read_len + bytesTransfered >= total_len) {
				//长度够了就调用回调函数
				handler(ec, read_len + bytesTransfered);
				return;
			}

			//没有错误，且长度不足则继续读取
			self->asyncReadLen(read_len + bytesTransfered, total_len, handler);
	});
}

LogicNode::LogicNode(shared_ptr<CSession> session, shared_ptr<RecvNode> recvnode)
	: _session(session), _recvnode(recvnode) {
	
}
