#pragma once
#include<unordered_map>
#include<string>
//rpcservice_ip  rpcservice_port   zookeeper_ip   zookeeper_port
class MprpcConfig{

public:
    void LoadConfigFile(const char* config_file);
    //查询配置项信息
    std::string Load(std::string key);
    
private:
    std::unordered_map<std::string,std::string>  m_configMap;
    void Trim(std::string &src_buf);
};