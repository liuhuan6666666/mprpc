#include <iostream>
#include "../user.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"
/*
    UserService 原来是一个本地服务，提供两个进程内的本地方法，login和GetFriendLists
*/
class UserService : public fixbug::UserServiceRpc // 使用在rpc服务发布端（rpc服务提供者）   
{
public:
    bool Login(std::string name, std::string pwd)
    {
        std::cout << "doing local service : login" << std::endl;
        std::cout << "name:" << name << " pwd:" << pwd << std::endl;
        return false;
    }
    bool Register(uint32_t id, std::string name, std::string pwd)
    {
        std::cout << "doing local service : register" << std::endl;
        std::cout <<"id: "<<id<< " name:" << name << " pwd:" << pwd << std::endl;
        return false;
    }
    /*
    重写基类虚函数 下面这些方法都是框架直接调用
    1.caller (调用者) ==> Login(LoginRequest) ==> muduo =>callee (被调用者)   序列化和反序列化都是框架自动完成
    2.callee (被调用者)==> Login(LoginRequest) => 交到下面重写的这个Login上 Login函数是框架帮忙调用
    */
    void Login(::google::protobuf::RpcController *controller,
               const ::fixbug::LoginRequest *request,
               ::fixbug::LoginResponse *response,
               ::google::protobuf::Closure *done)
    {
        // 框架给业务上报了请求参数LoginRequest,应用获取相应数据做本地业务
        std::string name = request->name();
        std::string pwd = request->pwd();
        // 做本地业务
        bool login_result = Login(name, pwd);
        // 把响应写入  包括错误码、错误消息、 返回值
        fixbug::ResultCode *code = response->mutable_result(); // 当参数为对象时，需要通过mutable_result获取
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_success(login_result);
        // 执行回调函数  主要完成响应对象数据的序列化和网络发送（由网络框架完成）
        done->Run();
    }
    void Register(::google::protobuf::RpcController *controller,
                  const ::fixbug::RegisterRequest *request,
                  ::fixbug::RegisterResponse *response,
                  ::google::protobuf::Closure *done)
    {
        // 框架给业务上报了请求参数LoginRequest,应用获取相应数据做本地业务
        std::string name = request->name();
        std::string pwd = request->pwd();
        uint32_t id = request->id();
        // 做本地业务
        bool register_result = Register(id,name, pwd);
        // 把响应写入  包括错误码、错误消息、 返回值
        fixbug::ResultCode *code = response->mutable_result(); // 当参数为对象时，需要通过mutable_result获取
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_success(register_result);
        // 执行回调函数  主要完成响应对象数据的序列化和网络发送（由网络框架完成）
        done->Run();
    }
};

int main(int argc, char **argv)
{
    // 调用框架初始化操作
    MprpcApplication::Init(argc, argv);

    // 把UserService服务对象发布到rpc节点上，可以上传任意的服务对象
    RpcProvider provider; // rpc网络服务对象  具有两个功能，一个是muduo库网络服务器，一个是
    provider.NotifyService(new UserService());

    // 启动一个rpc服务发布节点  Run()以后，进程进入阻塞状态，等到远程rpc调用请求
    provider.Run();
    return 0;
}
