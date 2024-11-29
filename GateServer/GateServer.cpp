#include "defs.h"
#include "CServer.h"
#include "ConfigMgr.h"
#include "RedisMgr.h"

void TestRedis() {
	//����redis����Ҫ�����ſ��Խ�������
	//redisĬ�ϼ����˿�Ϊ6379�������������ļ����޸�
	redisContext *c = redisConnect("172.25.0.50", 6380);
	if (c->err)
	{
		printf("Connect to redisServer failed:%s\n", c->errstr);
		redisFree(c);
		return;
	}
	printf("Connect to redisServer Success\n");

	//std::string redis_password = "123456";
	const char *redis_password = "123456";
	redisReply* r = (redisReply*)redisCommand(c, "AUTH %s", redis_password);
	if (r->type == REDIS_REPLY_ERROR) {
		printf("Redis��֤ʧ�ܣ�\n");
		return;
	}
	else {
		printf("Redis��֤�ɹ���\n");
	}

	//Ϊredis����key
	const char* command1 = "set stest1 value1";

	//ִ��redis������
	r = (redisReply*)redisCommand(c, command1);

	//�������NULL��˵��ִ��ʧ��
	if (NULL == r)
	{
		printf("Execut command1 failure\n");
		redisFree(c);
		return;
	}

	//���ִ��ʧ�����ͷ�����
	if (!(r->type == REDIS_REPLY_STATUS && (strcmp(r->str, "OK") == 0 || strcmp(r->str, "ok") == 0)))
	{
		printf("Failed to execute command[%s]\n", command1);
		freeReplyObject(r);
		redisFree(c);
		return;
	}

	//ִ�гɹ� �ͷ�redisCommandִ�к󷵻ص�redisReply��ռ�õ��ڴ�
	freeReplyObject(r);
	printf("Succeed to execute command[%s]\n", command1);

	const char* command2 = "strlen stest1";
	r = (redisReply*)redisCommand(c, command2);

	//����������Ͳ������� ���ͷ�����
	if (r->type != REDIS_REPLY_INTEGER)
	{
		printf("Failed to execute command[%s]\n", command2);
		freeReplyObject(r);
		redisFree(c);
		return;
	}

	//��ȡ�ַ�������
	int length = r->integer;
	freeReplyObject(r);
	printf("The length of 'stest1' is %d.\n", length);
	printf("Succeed to execute command[%s]\n", command2);

	//��ȡredis��ֵ����Ϣ
	const char* command3 = "get stest1";
	r = (redisReply*)redisCommand(c, command3);
	if (r->type != REDIS_REPLY_STRING)
	{
		printf("Failed to execute command[%s]\n", command3);
		freeReplyObject(r);
		redisFree(c);
		return;
	}
	printf("The value of 'stest1' is %s\n", r->str);
	freeReplyObject(r);
	printf("Succeed to execute command[%s]\n", command3);

	const char* command4 = "get stest2";
	r = (redisReply*)redisCommand(c, command4);
	if (r->type != REDIS_REPLY_NIL)
	{
		printf("Failed to execute command[%s]\n", command4);
		freeReplyObject(r);
		redisFree(c);
		return;
	}
	freeReplyObject(r);
	printf("Succeed to execute command[%s]\n", command4);

	//�ͷ�������Դ
	redisFree(c);

}

void TestRedisMgr() {
	//assert(RedisMgr::GetInstance()->Connect("172.25.0.50", 6380));
	assert(RedisMgr::GetInstance()->Auth("123456"));

	assert(RedisMgr::GetInstance()->Set("test0", "test0value"));

	std::string value = "";
	assert(RedisMgr::GetInstance()->Get("test0", value));
	assert(RedisMgr::GetInstance()->Del("test0"));
	assert(RedisMgr::GetInstance()->Get("test0", value) == false);

	//HSET key field value
	//HGETALL key
	assert(RedisMgr::GetInstance()->HSet("Hashkey", "field0", "value0"));
	assert(RedisMgr::GetInstance()->HGet("Hashkey", "field0") != "");
	assert(RedisMgr::GetInstance()->ExistKey("Hashkey"));
	assert(RedisMgr::GetInstance()->Del("Hashkey"));
	assert(RedisMgr::GetInstance()->Del("Hashkey"));
	assert(RedisMgr::GetInstance()->ExistKey("Hashkey") == false);

	//lpushkey1: lpushvalue3, lpushvalue2, lpushvalue1
	assert(RedisMgr::GetInstance()->LPush("lpushkey1", "lpushvalue1"));
	assert(RedisMgr::GetInstance()->LPush("lpushkey1", "lpushvalue2"));
	assert(RedisMgr::GetInstance()->LPush("lpushkey1", "lpushvalue3"));

	//lpushkey1: lpushvalue3, lpushvalue2
	assert(RedisMgr::GetInstance()->RPop("lpushkey1", value));
	//lpushkey1: lpushvalue3
	assert(RedisMgr::GetInstance()->RPop("lpushkey1", value));
	//lpushkey1: (empty)
	assert(RedisMgr::GetInstance()->LPop("lpushkey1", value));
	assert(RedisMgr::GetInstance()->LPop("lpushkey2", value) == false);

	RedisMgr::GetInstance()->Close();
}

int main() {
	//Redis test
	TestRedis();
	TestRedisMgr();

	ConfigMgr &gCfgMgr = ConfigMgr::init();
	
	std::string gateServerPort_str = gCfgMgr["GateServer"]["port"];
	unsigned short gateServerPort = atoi(gateServerPort_str.c_str());

	std::cout << "GateServer Port = " << gateServerPort << std::endl;

	try {
		unsigned short port = static_cast<unsigned short>(gateServerPort);
		net::io_context ioc{ 1 };
		boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
		signals.async_wait([&ioc](const boost::system::error_code &err, int sig_id) {
			if (err) {
				return;
			}
			
			ioc.stop();
		});

		std::make_shared<CServer>(ioc, port)->start();
		std::cout << "Gate Server listen on port " << port << std::endl;
		ioc.run();
	}
	catch (const std::exception &e){
		std::cerr << "GateServer Error " << e.what() << std::endl;
		return EXIT_FAILURE;
	}


	return 0;
}