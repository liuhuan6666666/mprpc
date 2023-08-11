#include <iostream>
#include "../../src/include/mprpcapplication.h"
#include "../friend.pb.h"

int main(int argc, char **argv)
{
    // 整个程序启动后，想使用mprpc框架享受rpc服务调用，一点更要先调用框架的初始化函数
    MprpcApplication::Init(argc, argv);
    // 调用远程发布的rpc方法login
    fixbug::FriendServiceRpc_Stub stub(new MprpcChannel()); // rpc服务使用者者需要使用UserServiceRpc_Stub才行
    // rpc请求参数  登录
    fixbug::GetFriendsListRequest request;
    request.set_userid(1);
    // rpc响应参数
    fixbug::GetFriendsListResponse response;
    // 定义一个controller对象
    MprpcController controller; // 可以知道调用过程中的状态信息

    // 发起rpc方法的调用，同步rpc调用过程
    stub.GetfriendList(&controller, &request, &response, nullptr); // 底层代码是：调用RPCChannel->CallMethod()函数，集中来做所有rpc方法调用的参数序列化和网络发送
    // 一次性rpc调用完成，读调用的结果
    if (controller.Failed())
    {
        std::cout << controller.ErrorText() << std::endl;
    }
    else
    {
        if (response.result().errcode() == 0)
        {
            std::cout << "rpc Getfriendlist response success!" << std::endl;
            int fri_size = response.friends_size();
            for (int i = 0; i < fri_size; i++)
            {
                std::cout << "index: " << i + 1 << " name: " << response.friends(i) << std::endl;
            }
        }
        else
        {
            std::cout << "rpc Getfriendlist response error: " << response.result().errmsg() << std::endl;
        }
    }

    return 0;
}