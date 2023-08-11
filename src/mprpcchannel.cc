#include "./include/mprpcchannel.h"
#include <string>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <mprpcapplication.h>
#include<hiredis/sockcompat.h>
#include "./include/mprpccontroller.h"
#include"logger.h"
/*
header_size + service_name + method_name + args_size +args
*/

// 所有通过stub代理对象调用的rpc方法，都走到这里了，统一做rpc方法调用的数据序列化和网络发送
void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor *method,
                              google::protobuf::RpcController *controller, const google::protobuf::Message *request,
                              google::protobuf::Message *response, google::protobuf::Closure *done)
{   
    const google::protobuf::ServiceDescriptor *sd = method->service();
    std::string service_name = sd->name();
    std::string method_name = method->name();

    int args_size = 0; // 获取参数序列化字符串长度
    std::string args_str;
    if (request->SerializePartialToString(&args_str))
    {
        args_size = args_str.size();
    }
    else
    {
        controller->SetFailed("Serialize request error!");
        return;
    }

    // 定义rpc的请求header
    mprpc::RpcHeader rpcheader;
    rpcheader.set_service_name(service_name);
    rpcheader.set_method_name(method_name);
    rpcheader.set_args_size(args_size);

    uint32_t header_size = 0; // 数据头长度
    std::string rpc_header_str;
    if (rpcheader.SerializePartialToString(&rpc_header_str))
    {
        header_size = rpc_header_str.size();
    }
    else
    {   
        controller->SetFailed("Serialize rpc header error!");
        return;
    }

    // 组合待发送rpc字符串
    std::string send_rpc_str;
    send_rpc_str.insert(0, std::string((char *)&header_size, 4)); // 从0位置写4个字节，二进制表示的整数
    send_rpc_str += rpc_header_str;
    send_rpc_str += args_str;
    // 打印调试的信息
    std::cout << "=============================================" << std::endl;
    std::cout << "header_size: " << header_size << std::endl;
    std::cout << "rpc_header_str: " << rpc_header_str << std::endl;
    std::cout << "header_size: " << header_size << std::endl;
    std::cout << "service_name: " << service_name << std::endl;
    std::cout << "method_name: " << method_name << std::endl;
    std::cout << "args_size: " << args_size << std::endl;
    std::cout << "args_atr: " << args_str << std::endl;
    std::cout << "=============================================" << std::endl;

    // 使用tcp编程，完成rpc方法的完成
    int connfd = socket(AF_INET, SOCK_STREAM, 0);
    if (connfd == -1)
    {   
        char errtxt[512];
        sprintf(errtxt,"errno:%d",errno);
        controller->SetFailed(errtxt);
        return;
    }
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    unsigned int num = 0;
    // std::string ip = MprpcApplication::GetInstance().Getconfig().Load("rpcserverip");
    // uint16_t port = atoi(MprpcApplication::GetInstance().Getconfig().Load("rpcserverport").c_str());
    ZkClient zkcli;
    zkcli.Start();
    std::string method_path="/"+service_name+"/"+method_name;
    std::string host_data=zkcli.GetDate(method_path.c_str());
    if(host_data==""){
        controller->SetFailed(method_path+" is not exist!");
        return;
    }
    int idx=host_data.find(":");
    if(idx==-1){
        controller->SetFailed(method_path+" address is invalid!");
        return;
    }
    std::string ip=host_data.substr(0,idx);
    uint64_t port=atoi(host_data.substr(idx+1,host_data.size()-idx-1).c_str());
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    server_addr.sin_port = htons(port);
    if (connect(connfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {   
        char errtxt[512];
        sprintf(errtxt,"connect error! errno:%d",errno);
        controller->SetFailed(errtxt);
        close(connfd);
        return;
    }

    if (send(connfd, send_rpc_str.c_str(), send_rpc_str.size(), 0) == -1)
    {
        char errtxt[512];
        sprintf(errtxt,"send error! errno:%d",errno);
        controller->SetFailed(errtxt);
        return;
    }

    // 接收rpc请求响应
    char recv_buf[1024];
    int recv_size = 0;
    if (-1 == (recv_size = recv(connfd, recv_buf, 1024, 0)))
    {
        char errtxt[1024];
        sprintf(errtxt,"recv error! errno:%d",errno);
        controller->SetFailed(errtxt);
        close(connfd);
        return;
    }
    // 反序列化rpc调用的响应数据
    //std::string response_str(buf, 0, recv_size); //bug出现问题：，recv_buf中遇到\0后面的数据就存不下来了
    if (!response->ParseFromArray(recv_buf,recv_size))  //反序列化成功返回true
    {   
        char errtxt[2048];
        sprintf(errtxt,"parse error! response_str:%s",recv_buf);
        controller->SetFailed(errtxt);
        close(connfd);
        return;
    }
    close(connfd);
}
