#include "mprpcapplication.h"
#include<iostream>
#include<unistd.h>
#include<string>
MprpcConfig MprpcApplication::m_config;
void ShowArgsHelp(){
    std::cout<<"format: command -i <configfile>"<<std::endl;  //<configfile>:配置文件名字
}
void MprpcApplication::Init(int argc,char **argv){
    if(argc<2){
        ShowArgsHelp();
        exit(EXIT_FAILURE);
    }
    int c=0;
    std::string config_file;
    while((c=getopt(argc,argv,"i:"))!=-1){ //"i:":表示必须出现i
        switch (c)
        {
        case 'i':
            config_file=optarg;
            break;
        case '?':   //"?":表示必须出现不想要的参数
            ShowArgsHelp();
            exit(EXIT_FAILURE);
        case ':': //"：":表示必须出现-i但是没有参数
            ShowArgsHelp();
            exit(EXIT_FAILURE);
        default:
            break;
        }
    }
    //开始加载配置文件  rpcservice_ip=  rpcservice_port=   zookeeper_ip=   zookeeper_port=
    m_config.LoadConfigFile(config_file.c_str());  //读取配置文件
}

MprpcApplication& MprpcApplication::GetInstance()
{
    static MprpcApplication _mprpcapplication;
    return _mprpcapplication;
}
 MprpcConfig& MprpcApplication::Getconfig(){
    return m_config;
 }