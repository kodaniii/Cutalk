#include "ConfigMgr.h"

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
ConfigMgr::ConfigMgr() {
	filesys::path cur_path = filesys::current_path();
	filesys::path config_path = cur_path / "config.ini";

	std::cout << "config_path = " << config_path << std::endl;

	boost::property_tree::ptree pt;
	boost::property_tree::read_ini(config_path.string(), pt);

    //database、server
	for (const auto& _pt_pair : pt) {
        const std::string& config_serve_name = _pt_pair.first;   //database、server
        const boost::property_tree::ptree config_serve_info_pt = _pt_pair.second;

        std::map<std::string, std::string> config_serve_info;
        for (const auto& _config_serve_info_pair : config_serve_info_pt) {
            //key = host, value = localhost
            const std::string& key = _config_serve_info_pair.first;
            const std::string& value = _config_serve_info_pair.second.get_value<std::string>();  //.second is ptree
        
            config_serve_info[key] = value;
        }
        
        //for database or server
        ConfigInfo configInfo;
        configInfo._config_data = config_serve_info;

        this->_config_info[config_serve_name] = configInfo;
	}

    for (auto const& _config_info_pair : _config_info) {
        const std::string& _config_info_name = _config_info_pair.first;
        const ConfigInfo& _config_info_key_value = _config_info_pair.second;

        std::cout << "[" << _config_info_name << "]" << std::endl;
        
        for (auto const& _config_info_key_value_pair : _config_info_key_value._config_data) {
            std::cout << _config_info_key_value_pair.first << " = " << _config_info_key_value_pair.second << std::endl;
        }
    }
}

/*
[PeerServers]
name = ChatServer2
[Redis]
host = 172.25.0.50
port = 6380
pswd = 123456
[SelfServer]
RPCPort = 50055
host = 127.0.0.1
name = ChatServer1
port = 8090
[StatusServer]
host = 127.0.0.1
port = 50052
[VerifyServer]
host = 127.0.0.1
port = 50051

if section VerifyServer, key = host, return 127.0.0.1
*/

std::string ConfigMgr::GetValue(const std::string& section, const std::string& key) {
    if (_config_info.find(section) == _config_info.end()) {
        return "";
    }

    return _config_info[section].GetValue(key);
}