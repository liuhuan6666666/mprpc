#include <iostream>
#include "../friend.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"
#include"logger.h"
#include <vector>
class FriendService : public fixbug::FriendServiceRpc
{
public:
    std::vector<std::string> GetFriendList(uint32_t userid)
    {
        std::cout << "do GetFriendList service! userid: "<<userid<<std::endl;
        std::vector<std::string> vec;
        vec.push_back("gao yang");
        vec.push_back("liu hong");
        vec.push_back("li si");
        return vec;
    }
    //下面函数是将父类中函数重载
    void GetfriendList(::google::protobuf::RpcController *controller,
                       const ::fixbug::GetFriendsListRequest *request,
                       ::fixbug::GetFriendsListResponse *response,
                       ::google::protobuf::Closure *done)
    {
        uint32_t userid=request->userid();
        std::vector<std::string> friendList=GetFriendList(userid);
        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        for(std::string &name : friendList){
            std::string *p=response->add_friends();
            *p=name;
        }
        done->Run();
    }
};
int main(int argc, char **argv)
{   
    LOG_INFO("frist log message!");
    LOG_ERR("%s:%s:%d",__FILE__,__FUNCTION__,__LINE__);
    // 调用框架初始化操作
    MprpcApplication::Init(argc, argv);

    // 把UserService服务对象发布到rpc节点上，可以上传任意的服务对象
    RpcProvider provider; // rpc网络服务对象  具有两个功能，一个是muduo库网络服务器，一个是
    provider.NotifyService(new FriendService());

    // 启动一个rpc服务发布节点  Run()以后，进程进入阻塞状态，等到远程rpc调用请求
    provider.Run();
    return 0;
}
