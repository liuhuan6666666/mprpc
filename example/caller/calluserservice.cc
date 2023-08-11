#include<iostream>
#include"../../src/include/mprpcapplication.h"
#include"../user.pb.h"
#include"../../src/include/mprpcchannel.h"
int main(int argc,char** argv){
    //整个程序启动后，想使用mprpc框架享受rpc服务调用，一点更要先调用框架的初始化函数
    MprpcApplication::Init(argc,argv);
    //调用远程发布的rpc方法login       MprpcChannel():代理对象，帮它对数据序列化和反序列化以及网络收发
    fixbug::UserServiceRpc_Stub stub(new MprpcChannel());     //rpc服务使用者者需要使用UserServiceRpc_Stub才行
    //rpc请求参数  登录
    fixbug::LoginRequest login_request;
    login_request.set_name("zhang san");
    login_request.set_pwd("123456");
    //rpc响应参数
    fixbug::LoginResponse login_response;
    //发起rpc方法的调用，同步rpc调用过程    
    stub.Login(nullptr,&login_request,&login_response,nullptr);  //底层代码是：调用RPCChannel->CallMethod()函数，集中来做所有rpc方法调用的参数序列化和网络发送
    //一次性rpc调用完成，读调用的结果
    if(login_response.result().errcode()==0){
        std::cout<<"rpc login response success: "<<login_response.success()<<std::endl;
    }else{
        std::cout<<"rpc login response error: "<<login_response.result().errmsg()<<std::endl;
    }

    //rpc请求参数 注册
    fixbug::RegisterRequest register_request;
    register_request.set_name("zhang san");
    register_request.set_pwd("123456");
    register_request.set_id(1);
    //rpc响应参数
    fixbug::RegisterResponse register_response;
    stub.Register(nullptr,&register_request,&register_response,nullptr);

    //一次性rpc调用完成，读调用的结果
    if(register_response.result().errcode()==0){
        std::cout<<"rpc register response success: "<<register_response.success()<<std::endl;
    }else{
        std::cout<<"rpc register response error: "<<register_response.result().errmsg()<<std::endl;
    }
    return 0;
}