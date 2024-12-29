#pragma once

#include "defs.h"
/*
[database]
	host = localhost
	port = 8080
	username = admin
	password = admin123
[server]
	port = 50051
	address = 192.168.1.1
	timeout = 60
*/

//key = host, value = localhost
struct ConfigInfo {
	ConfigInfo() {}

	//std::map默认支持深拷贝，不需要手动实现深拷贝
	//ConfigInfo(const ConfigInfo& c) {
	//	_config_data = c._config_data;
	//}
	ConfigInfo(ConfigInfo const& c) = default;

	~ConfigInfo() {
		_config_data.clear();
	}

	std::map<std::string, std::string> _config_data;

	ConfigInfo& operator=(ConfigInfo const& c) {
		if (&c == this) {
			return *this;
		}

		this->_config_data = c._config_data;
	}

	std::string operator[](std::string const& key) {
		if (_config_data.find(key) == _config_data.end()) {
			return "";
		}
		return _config_data[key];
	}

	std::string GetValue(const std::string& key) {
		if (_config_data.find(key) == _config_data.end()) {
			return "";
		}

		return _config_data[key];
	}

};


//key = database, value = class ConfigInfo(...)
class ConfigMgr
{
public:
	ConfigMgr(ConfigMgr const& c) = delete;// default;
	
	~ConfigMgr() {
		_config_info.clear();
	}

	/*
	ConfigMgr operator=(ConfigMgr const& src) {
		if (&src == this) {
			return *this;
		}
		this->_config_info = src._config_info;
		return *this;
	}*/

	ConfigInfo operator[](std::string const& key) {
		if (_config_info.find(key) == _config_info.end()) {
			return ConfigInfo();
		}
		return _config_info[key];
	}
	
	static ConfigMgr& init() {
		static ConfigMgr cfg_mgr;
		return cfg_mgr;
	}

	std::string GetValue(const std::string& section, const std::string& key);

private:
	ConfigMgr();
	std::map<std::string, ConfigInfo> _config_info;
};



