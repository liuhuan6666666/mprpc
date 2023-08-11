#pragma once
#include<memory>
#include<muduo/net/TcpServer.h>
#include<muduo/net/EventLoop.h>
#include<muduo/net/InetAddress.h>
#include<muduo/net/TcpConnection.h>
#include "google/protobuf/service.h"
#include<google/protobuf/descriptor.h>
#include<unordered_map>
#include<logger.h>
#include"zookeeperutil.h"
//框架提供的专门服务发布rpc服务的网络对象类
class RpcProvider{
public:
    //这里是框架提供给外部使用的，可以发布rpc方法的函数接口     需要接收任意的service   多态
    void  NotifyService(google::protobuf::Service* service);

    //启动rpc服务节点，开始提供rpc远程调用服务  调用muduo库
    void Run();
private:
    //组合了EVentloop
    muduo::net::EventLoop m_eventloop;
    //service服务类型信息
    struct ServiceInfo
    {
        google::protobuf::Service* m_service;//保存服务对象
        std::unordered_map<std::string,const google::protobuf::MethodDescriptor*> m_methodMap;//服务方法
    };
    std::unordered_map<std::string,ServiceInfo> m_serviceMap;//存储注册成功的服务方法和其服务方法的所有信息
    void Onconnext(const muduo::net::TcpConnectionPtr&);
    void OnMessage(const muduo::net::TcpConnectionPtr&,muduo::net::Buffer*,muduo::Timestamp);
    //closure回调操作，用于序列化rpc的响应和网络发送
    void SendRpcResponse(const muduo::net::TcpConnectionPtr&,google::protobuf::Message*);

};