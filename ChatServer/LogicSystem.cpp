#include "LogicSystem.h"
#include "RedisMgr.h"
#include "MysqlMgr.h"
#include "defs.h"
#include "StatusGrpcClient.h"
#include "UserMgr.h"
#include "ChatGrpcClient.h"

#define forever for(;;)

using namespace std;

LogicSystem::LogicSystem()
	: _isStop(false) {
	std::cout << "LogicSystem::LogicSystem()" << std::endl;
	RegisterCallBacks();
	_work_thread = std::thread(&LogicSystem::DealMsg, this);
}

LogicSystem::~LogicSystem() {
	_isStop = true;
	_cond.notify_one();
	_work_thread.join();
}

void LogicSystem::PostMsgToQue(shared_ptr<LogicNode> msg) {
	std::unique_lock<std::mutex> lk(_mtx);
	//消息放到队列中
	_msg_que.push(msg);

	//如果_msg_que原本是空的，现在放入了一个，说明线程被条件变量阻塞，唤醒线程处理
	if (_msg_que.size() == 1) {
		lk.unlock();
		_cond.notify_one();
	}
}

//处理队列中的消息
void LogicSystem::DealMsg() {
	std::cout << "LogicSystem::DealMsg()" << std::endl;
	forever{
		std::unique_lock<std::mutex> lk(_mtx);
	//判断队列为空，条件变量阻塞等待，并释放锁
	//直到析构时唤醒或队列被放入消息时唤醒
	while (_msg_que.empty() && !_isStop) {
		_cond.wait(lk);
	}

	//关闭状态，把剩余消息全部发送
	if (_isStop) {
		while (!_msg_que.empty()) {
			auto msg_node = _msg_que.front();
			cout << "_msg_que.front() msg_type_id = " << msg_node->_recvnode->_msg_type_id << endl;
			auto call_back_iter = _func_callbacks.find(msg_node->_recvnode->_msg_type_id);

			//没找到对应的回调函数，直接queue.pop()
			if (call_back_iter == _func_callbacks.end()) {
				std::cout << "cannot found _func_callbacks[msg_type_id]" << std::endl;
				_msg_que.pop();
				continue;
			}

			//调用对应的回调函数
			call_back_iter->second(msg_node->_session, msg_node->_recvnode->_msg_type_id,
				std::string(msg_node->_recvnode->_data, msg_node->_recvnode->_cur_len));
			_msg_que.pop();
		}
		break;
	}

	//队列中有数据
	auto msg_node = _msg_que.front();
	cout << "_msg_que.front() msg_type_id = " << msg_node->_recvnode->_msg_type_id << endl;
	auto call_back_iter = _func_callbacks.find(msg_node->_recvnode->_msg_type_id);

	if (call_back_iter == _func_callbacks.end()) {
		std::cout << "cannot found _func_callbacks msg_type_id "
				<< msg_node->_recvnode->_msg_type_id << std::endl;
		_msg_que.pop();
		continue;
	}

	call_back_iter->second(msg_node->_session, msg_node->_recvnode->_msg_type_id,
		std::string(msg_node->_recvnode->_data, msg_node->_recvnode->_cur_len));
	_msg_que.pop();
	}
}

void LogicSystem::RegisterCallBacks() {
	_func_callbacks[REQ_CHAT_LOGIN] = std::bind(&LogicSystem::LoginHandler, this,
		placeholders::_1, placeholders::_2, placeholders::_3);
	_func_callbacks[REQ_SEARCH_USER] = std::bind(&LogicSystem::SearchInfo, this,
		placeholders::_1, placeholders::_2, placeholders::_3);
	_func_callbacks[REQ_ADD_FRIEND_REQ] = std::bind(&LogicSystem::AddFriendApply, this,
		placeholders::_1, placeholders::_2, placeholders::_3);
}

/* 查询用户信息
 * 先查询Redis，如果没有再查询Mysql
 */
bool LogicSystem::GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo)
{
	//优先查redis中查询用户信息
	std::string info_str = "";
	bool b_base = RedisMgr::GetInstance()->Get(base_key, info_str);
	if (b_base) {
		Json::Reader reader;
		Json::Value root;
		reader.parse(info_str, root);
		userinfo->uid = root["uid"].asInt();
		userinfo->name = root["name"].asString();
		userinfo->pswd = root["pswd"].asString();
		userinfo->email = root["email"].asString();
		userinfo->nick = root["nick"].asString();
		userinfo->desc = root["desc"].asString();
		userinfo->sex = root["sex"].asInt();
		userinfo->icon = root["icon"].asString();
		std::cout << "user login uid " << userinfo->uid << ", name "
			<< userinfo->name << ", pswd " << userinfo->pswd << ", email " << userinfo->email << endl;
	}
	else {
		//redis中没有则查询mysql
		//查询数据库
		std::shared_ptr<UserInfo> user_info = nullptr;
		user_info = MysqlMgr::GetInstance()->GetUser(uid);
		if (user_info == nullptr) {
			return false;
		}

		userinfo = user_info;

		//将数据库内容写入redis缓存
		Json::Value redis_root;
		redis_root["uid"] = uid;
		redis_root["pswd"] = userinfo->pswd;
		redis_root["name"] = userinfo->name;
		redis_root["email"] = userinfo->email;
		redis_root["nick"] = userinfo->nick;
		redis_root["desc"] = userinfo->desc;
		redis_root["sex"] = userinfo->sex;
		redis_root["icon"] = userinfo->icon;
		RedisMgr::GetInstance()->Set(base_key, redis_root.toStyledString());
	}

	return true;
}

//用户登录处理，获取联系人、好友申请等各种信息
void LogicSystem::LoginHandler(shared_ptr<CSession> session, const short& msg_type_id, const string& msg_data) {
	std::cout << "LogicSystem::LoginHandler()" << std::endl;

	Json::Reader reader;
	Json::Value root;
	reader.parse(msg_data, root);
	auto uid = root["uid"].asInt();
	auto token = root["token"].asString();
	std::cout << "user login uid " << uid << ", user token " << token << endl;

	Json::Value rtvalue;
	Defer defer([this, &rtvalue, session]() {
		std::string return_str = rtvalue.toStyledString();	//序列化
		session->Send(return_str, REQ_CHAT_LOGIN_RSP);	//发到发送队列
		});

	//从redis获取用户token是否正确
	std::string uid_str = std::to_string(uid);
	std::string token_key = USER_TOKEN_PREFIX + uid_str;
	std::string token_value = "";
	bool success = RedisMgr::GetInstance()->Get(token_key, token_value);
	if (!success) {
		rtvalue["error"] = ErrorCodes::UidInvalid;
		return;
	}

	if (token_value != token) {
		rtvalue["error"] = ErrorCodes::TokenInvalid;
		return;
	}

	rtvalue["error"] = ErrorCodes::Success;


	/*获取用户个人信息*/
	// TODO GateServer中register和reset后也要更新，因为GetBaseInfo在Redis存在时只读Redis，而reg和reset只更新了Mysql，
	// 导致只有Mysql才是最新的，Redis不一定是最新的，还没做
	std::string base_key = USER_BASE_INFO + uid_str;
	auto user_info = std::make_shared<UserInfo>();
	bool b_base = GetBaseInfo(base_key, uid, user_info);
	if (!b_base) {
		rtvalue["error"] = ErrorCodes::UidInvalid;
		return;
	}

	rtvalue["uid"] = uid;
	rtvalue["pswd"] = user_info->pswd;
	rtvalue["name"] = user_info->name;
	rtvalue["email"] = user_info->email;
	rtvalue["nick"] = user_info->nick;
	rtvalue["desc"] = user_info->desc;
	rtvalue["sex"] = user_info->sex;
	rtvalue["icon"] = user_info->icon;


	//TODO 获取好友列表

	//TODO 获取好友申请列表, is ok
	//从Mysql获取
	std::vector<std::shared_ptr<ApplyInfo>> apply_list;
	auto b_apply = GetFriendApplyInfo(uid, apply_list);
	if (b_apply) {
		for (auto& apply : apply_list) {
			Json::Value obj;
			obj["name"] = apply->_name;
			obj["uid"] = apply->_uid;
			obj["icon"] = apply->_icon;
			obj["nick"] = apply->_nick;
			obj["sex"] = apply->_sex;
			obj["desc"] = apply->_desc;
			obj["status"] = apply->_status;
			rtvalue["apply_list"].append(obj);
		}
	}


	/* 更新服务器登录数量，供StatusServer负载均衡使用
	 * 服务器数量在服务器启动的时候就初始化了
	 */
	auto server_name = ConfigMgr::init().GetValue("SelfServer", "name");

	/*
	auto rd_res = RedisMgr::GetInstance()->HGet(LOGIN_COUNT, server_name);
	int login_cnt = 0;
	if (!rd_res.empty()) {
		login_cnt = std::stoi(rd_res);
	}
	login_cnt++;
	auto count_str = std::to_string(login_cnt);
	RedisMgr::GetInstance()->HSet(LOGIN_COUNT, server_name, count_str);*/

	bool isSucc = RedisMgr::GetInstance()->HAdd(LOGIN_COUNT, server_name);
	std::cout << "RedisMgr::GetInstance()->HAdd() " << (isSucc ? "success" : "fail") << std::endl;


	/*绑定当前TCP session的用户uid*/
	session->SetUserId(uid);

	/* 将用户uid所在的ChatServer name保存到Redis，
	 * 用于定位用户聊天服务器所在ip和port，
	 * 便于跨服务器通信，对于直接找到目标用户的聊天服务器
	 */
	std::string uid_ip = USER_IP_PREFIX + uid_str;
	RedisMgr::GetInstance()->Set(uid_ip, server_name);

	/* 将uid和TCP session记录到UserMgr _uid_to_session中
	 * 实现uid到session映射，可以通过CServer::ClearSession(session_id)实现踢人(断开连接)功能
	 * CServer::ClearSession(session_id)清除了UserMgr中uid到session的映射，然后清除session本身
	 */
	UserMgr::GetInstance()->SetUserSession(uid, session);

	return;



	/*	//由于是多ChatServer，统一配置到Redis上，不存在本地内存中
	//从状态服务器获取token匹配是否准确
	//LoginReq(uid, token) -> LoginRsp(error, uid, token)
	auto rsp = StatusGrpcClient::GetInstance()->Login(uid, root["token"].asString());
	std::cout << "StatusGrpcClient Login rsp error " << rsp.error()
		<< ", uid " << rsp.uid() << ", token " << rsp.token() << std::endl;


	rtvalue["error"] = rsp.error();
	if (rsp.error() != ErrorCodes::Success) {
		return;
	}*/


	/*	//这部分代码封装到LogicSystem::GetBaseInfo()
	//内存中查询用户信息
	auto find_iter = _users.find(uid);
	std::shared_ptr<UserInfo> user_info = nullptr;
	//用户信息不在内存中
	if (find_iter == _users.end()) {
		//通过uid查询数据库
		user_info = MysqlMgr::GetInstance()->GetUser(uid);
		if (user_info == nullptr) {
			std::cout << "Cannot found uid " << uid << " in Mysql user table" << std::endl;
			rtvalue["error"] = ErrorCodes::UidInvalid;
			return;
		}

		_users[uid] = user_info;
	}
	else {
		user_info = find_iter->second;
	}

	rtvalue["uid"] = uid;
	rtvalue["token"] = rsp.token();
	rtvalue["name"] = user_info->name;*/

}

bool LogicSystem::isPureDigit(const std::string& str) {
	for (char c : str) {
		if (!std::isdigit(c)) {
			return false;
		}
	}
	return true;
}

void LogicSystem::GetUserByUid(std::string uid_str, Json::Value& rtvalue) {
	std::cout << "LogicSystem::GetUserByUid() uid " << uid_str << std::endl;

	std::string base_key = USER_BASE_INFO + uid_str;

	//优先从Redis中查询用户信息
	std::string info_str = "";
	bool b_base = RedisMgr::GetInstance()->Get(base_key, info_str);
	if (b_base) {
		Json::Reader reader;
		Json::Value root;
		reader.parse(info_str, root);

		auto uid = root["uid"].asInt();
		auto name = root["name"].asString();
		auto pswd = root["pswd"].asString();
		auto email = root["email"].asString();
		auto nick = root["nick"].asString();
		auto desc = root["desc"].asString();
		auto sex = root["sex"].asInt();
		auto icon = root["icon"].asString();

		std::cout << "user uid " << uid << ", name "
			<< name << ", pswd " << pswd << ", email "
			<< email << std::endl; // ", icon " << icon << std::endl;

		rtvalue["error"] = ErrorCodes::Success;
		rtvalue["uid"] = uid;
		rtvalue["pswd"] = pswd;
		rtvalue["name"] = name;
		rtvalue["email"] = email;
		rtvalue["nick"] = nick;
		rtvalue["desc"] = desc;
		rtvalue["sex"] = sex;
		rtvalue["icon"] = icon;
		return;
	}

	auto uid = std::stoi(uid_str);
	//redis中没有则查询mysql
	//查询数据库
	std::shared_ptr<UserInfo> user_info = nullptr;
	user_info = MysqlMgr::GetInstance()->GetUser(uid);
	if (user_info == nullptr) {
		rtvalue["error"] = ErrorCodes::UidInvalid;
		return;
	}

	//将数据库内容写入Redis缓存
	Json::Value redis_root;
	redis_root["uid"] = user_info->uid;
	redis_root["pswd"] = user_info->pswd;
	redis_root["name"] = user_info->name;
	redis_root["email"] = user_info->email;
	redis_root["nick"] = user_info->nick;
	redis_root["desc"] = user_info->desc;
	redis_root["sex"] = user_info->sex;
	redis_root["icon"] = user_info->icon;

	RedisMgr::GetInstance()->Set(base_key, redis_root.toStyledString());

	//返回数据
	rtvalue["error"] = ErrorCodes::Success;
	rtvalue["uid"] = user_info->uid;
	rtvalue["pswd"] = user_info->pswd;
	rtvalue["name"] = user_info->name;
	rtvalue["email"] = user_info->email;
	rtvalue["nick"] = user_info->nick;
	rtvalue["desc"] = user_info->desc;
	rtvalue["sex"] = user_info->sex;
	rtvalue["icon"] = user_info->icon;

	return;
}

void LogicSystem::GetUserByName(std::string name, Json::Value& rtvalue) {
	std::cout << "LogicSystem::GetUserByName() name " << name << std::endl;

	std::string base_key = NAME_INFO + name;

	//优先查redis中查询用户信息
	std::string info_str = "";
	bool b_base = RedisMgr::GetInstance()->Get(base_key, info_str);
	if (b_base) {
		Json::Reader reader;
		Json::Value root;
		reader.parse(info_str, root);
		auto uid = root["uid"].asInt();
		auto name = root["name"].asString();
		auto pswd = root["pswd"].asString();
		auto email = root["email"].asString();
		auto nick = root["nick"].asString();
		auto desc = root["desc"].asString();
		auto sex = root["sex"].asInt();
		auto icon = root["icon"].asString();

		std::cout << "user uid " << uid << ", name "
			<< name << ", pswd " << pswd << ", email "
			<< email << std::endl;//", icon " << icon << std::endl;

		rtvalue["error"] = ErrorCodes::Success;
		rtvalue["uid"] = uid;
		rtvalue["pswd"] = pswd;
		rtvalue["name"] = name;
		rtvalue["email"] = email;
		rtvalue["nick"] = nick;
		rtvalue["desc"] = desc;
		rtvalue["sex"] = sex;
		rtvalue["icon"] = icon;
		return;
	}

	//redis中没有则查询mysql
	//查询数据库
	std::shared_ptr<UserInfo> user_info = nullptr;
	user_info = MysqlMgr::GetInstance()->GetUser(name);
	if (user_info == nullptr) {
		rtvalue["error"] = ErrorCodes::UidInvalid;
		return;
	}

	//将数据库内容写入redis缓存
	Json::Value redis_root;
	redis_root["uid"] = user_info->uid;
	redis_root["pswd"] = user_info->pswd;
	redis_root["name"] = user_info->name;
	redis_root["email"] = user_info->email;
	redis_root["nick"] = user_info->nick;
	redis_root["desc"] = user_info->desc;
	redis_root["sex"] = user_info->sex;
	redis_root["icon"] = user_info->icon;

	RedisMgr::GetInstance()->Set(base_key, redis_root.toStyledString());

	//返回数据
	rtvalue["error"] = ErrorCodes::Success;
	rtvalue["uid"] = user_info->uid;
	rtvalue["pswd"] = user_info->pswd;
	rtvalue["name"] = user_info->name;
	rtvalue["email"] = user_info->email;
	rtvalue["nick"] = user_info->nick;
	rtvalue["desc"] = user_info->desc;
	rtvalue["sex"] = user_info->sex;

	return;
}

void LogicSystem::SearchInfo(std::shared_ptr<CSession> session,
	const short& msg_type_id, const string& msg_data) {
	std::cout << "LogicSystem::SearchInfo()" << std::endl;
	Json::Reader reader;
	Json::Value root;
	reader.parse(msg_data, root);
	auto uid_or_name_str = root["uid_or_name"].asString();
	std::cout << "Get search_edit " << uid_or_name_str << endl;

	Json::Value rtvalue;

	Defer defer([this, &rtvalue, session]() {
		std::string return_str = rtvalue.toStyledString();
		session->Send(return_str, REQ_SEARCH_USER_RSP);
		});

	bool b_digit = isPureDigit(uid_or_name_str);
	if (b_digit) {
		GetUserByUid(uid_or_name_str, rtvalue);
	}
	else {
		GetUserByName(uid_or_name_str, rtvalue);
	}
	return;
}


void LogicSystem::AddFriendApply(std::shared_ptr<CSession> session, const short& msg_type_id, const string& msg_data)
{
	std::cout << "LogicSystem::AddFriendApply()" << std::endl;
	Json::Reader reader;
	Json::Value root;
	/*发送方信息*/
	reader.parse(msg_data, root);
	auto send_uid = root["send_uid"].asInt();
	auto send_reason = root["send_reason"].asString();
	auto send_backname = root["send_backname"].asString();
	auto send_touid = root["send_touid"].asInt();
	auto send_name = root["send_name"].asString();

	std::cout << "Get User AddFriend Msg From uid " << send_uid << ", name " << send_name
		<< ", reason " << send_reason << ", backname " << send_backname
		<< ", add_uid " << send_touid << std::endl;

	Json::Value rtvalue;
	rtvalue["error"] = ErrorCodes::Success;
	Defer defer([this, &rtvalue, session]() {
		std::string return_str = rtvalue.toStyledString();
		session->Send(return_str, REQ_ADD_FRIEND_RSP);
		});

	/*将好友添加信息更新到Mysql数据库*/
	//记录发起者uid和接收者uid
	bool isSucc = MysqlMgr::GetInstance()->AddFriendApply(send_uid, send_touid, send_reason);

	if (!isSucc) {
		rtvalue["error"] = ErrorCodes::MysqlFailed;
		std::cout << "MysqlMgr::GetInstance()->AddFriendApply() ret false" << std::endl;
		return;
	}

	/* 找到接收者uid的ChatServer服务器
	 * 根据服务器是SelfServer还是非SelfServer，决定消息发送到本地ChatServer还是其他Server
	 */
	 //查找Redis中存储的touid对应的server ip
	auto touid_str = std::to_string(send_touid);
	auto to_ip_key = USER_IP_PREFIX + touid_str;
	std::string to_ip_value = "";
	bool b_ip = RedisMgr::GetInstance()->Get(to_ip_key, to_ip_value);
	//对方不在线的情况，之前已经存储到Mysql，直接success不操作
	if (!b_ip) {
		std::cout << "LogicSystem::AddFriendApply(): the other party is offline" << std::endl;
		rtvalue["error"] = ErrorCodes::Success;
		return;
	}

	auto& cfg = ConfigMgr::init();
	auto self_name = cfg["SelfServer"]["name"];

	std::string base_key = USER_BASE_INFO + std::to_string(send_uid);
	auto apply_info = std::make_shared<UserInfo>();

	bool b_info = GetBaseInfo(base_key, send_uid, apply_info);

	//如果对方Client所在ChatServer是本地
	if (to_ip_value == self_name) {
		std::cout << "LogicSystem::AddFriendApply(): Both parties are on the same ChatServer, using TCP" << std::endl;
		auto session = UserMgr::GetInstance()->GetSession(send_touid);
		//tcp连接不为空，说明对方在线，可以通过这个session直接通知对方Client
		if (session) {
			//在内存中则直接发送通知对方
			Json::Value notify;
			notify["error"] = ErrorCodes::Success;
			notify["send_uid"] = send_uid;
			notify["send_name"] = send_name;
			notify["desc"] = send_reason;
			if (b_info) {
				//发送方信息
				notify["icon"] = apply_info->icon;
				notify["sex"] = apply_info->sex;
				notify["nick"] = apply_info->nick;
			}
			std::string return_str = notify.toStyledString();
			session->Send(return_str, REQ_NOTIFY_ADD_FRIEND_REQ);
		}

		return;
	}

	std::cout << "LogicSystem::AddFriendApply(): Both parties are not on the same ChatServer, using gRPC" << std::endl;

	//如果对方Client所在ChatServer不在本地，通过gRPC通知对方的ChatServer
	AddFriendReq add_req;
	add_req.set_applyuid(send_uid);
	add_req.set_touid(send_touid);
	add_req.set_name(send_name);
	add_req.set_desc(send_reason);
	if (b_info) {
		//发送方信息
		add_req.set_icon(apply_info->icon);
		add_req.set_sex(apply_info->sex);
		add_req.set_nick(apply_info->nick);
	}

	//发送通知
	ChatGrpcClient::GetInstance()->NotifyAddFriend(to_ip_value, add_req);

}

bool LogicSystem::GetFriendApplyInfo(int to_uid, std::vector<std::shared_ptr<ApplyInfo>>& list) {
	std::cout << "LogicSystem::GetFriendApplyInfo()" << std::endl;
	//从mysql获取好友申请列表
	return MysqlMgr::GetInstance()->GetApplyList(to_uid, list, 0, 10);
}
