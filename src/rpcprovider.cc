#include "rpcprovider.h"
#include <mprpcapplication.h>
#include <functional>
#include "rpcheader.pb.h"
using namespace mprpc;

/*
service_name =>service描述
                        =>service* (记录服务对象)
                        =>method_name => mehtod方法对象
json protobuf
*/
// 这里是框架提供给外部使用的(就是将服务和服务方法放在map中，rpc可以通过这些方法调用相应函数)，可以发布rpc方法的函数接口    需要接收任意的service   多态
void RpcProvider::NotifyService(google::protobuf::Service *service)
{
    ServiceInfo serviceinfo;
    // 获取服务对象的描述
    const google::protobuf::ServiceDescriptor *pserviceDesc = service->GetDescriptor();
    // 获取服务名字
    const std::string service_name = pserviceDesc->name();
    // 获取服务对象的service的方法数量
    int methodCnt = pserviceDesc->method_count();

    LOG_INFO("service_name:%s",service_name.c_str());
    for (int i = 0; i < methodCnt; i++)
    {
        // 获取服务对象指定下标的服务方法的描述（抽象描述）
        const google::protobuf::MethodDescriptor *pmethodDesc = pserviceDesc->method(i); // 返回服务方法
        std::string method_name = pmethodDesc->name();
        serviceinfo.m_methodMap.insert({method_name, pmethodDesc}); // 增加服务方法
        LOG_INFO("method_name:%s",method_name.c_str());
    }
    serviceinfo.m_service = service;
    m_serviceMap.insert({service_name, serviceinfo}); // 加入服务
}

// 启动rpc服务节点，开始提供rpc远程调用服务
void RpcProvider::Run()
{
    std::string ip = MprpcApplication::GetInstance().Getconfig().Load("rpcserverip");
    uint16_t port = atoi(MprpcApplication::GetInstance().Getconfig().Load("rpcserverport").c_str());
    muduo::net::InetAddress address(ip, port);
    // 创建Tcpserver对象
    muduo::net::TcpServer server(&m_eventloop, address, "RpcProvider"); // 当该对象被销毁时，会在其析构函数中删除关联的原始指针,防止内存泄漏
    // 绑定连接回调和消息读写函数   muduo好处：分离了网络和业务代码
    server.setConnectionCallback(std::bind(&RpcProvider::Onconnext, this, std::placeholders::_1));
    server.setMessageCallback(std::bind(&RpcProvider::OnMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    // 设置muduo库线程
    server.setThreadNum(4);
    //把当前rpc节点上要发布的服务全部注册到zk上面，让rpc client可以从zk上发现服务
    ZkClient zkcli;
    zkcli.Start();  //zookeeper连接，因为连接是异步的，因此需要通过信号量（主线程信号量初始化为0）来控制程序堵塞，当wather检测到连接成功会将信号量加1，主线程继续运行
    //service_name 为永久性节点   method_name 为临时节点
    //seesion timeout 30s      zkclient API：网络I/O线程  1/3*timeout时间发送心跳消息
    for(auto &sp:m_serviceMap){
        //service_name
        std::string service_path="/"+sp.first;  //zookeeper只支持一层一层创建节点
        std::cout<<service_path<<std::endl;
        zkcli.Create(service_path.c_str(),nullptr,ZOO_SEQUENCE);//永久性节点
        for(auto &mp:sp.second.m_methodMap){
            //service_name/method_nem
            std::string method_path="/"+sp.first+"/"+mp.first;
            std::cout<<method_path<<std::endl;
            char method_path_data[128]; //当前rpc服务节点主机的ip和port
            sprintf(method_path_data,"%s:%d",ip.c_str(),port);
            zkcli.Create(method_path.c_str(),method_path_data,strlen(method_path_data),ZOO_EPHEMERAL);//临时性节点
        }
    }
    std::cout << "RpcProvider start service at ip: " << ip << " port:" << port << std::endl;
    // 启动网络服务
    server.start();
    m_eventloop.loop(); // 等待远程连接
}

void RpcProvider::Onconnext(const muduo::net::TcpConnectionPtr &conn)
{
    if (!conn->connected())
    {
        // 表示rpc client连接断开
        conn->shutdown();
    }
}
/*
在框架内部，rpcProvider和rpcConsumer协商好通信用的protobuf模型
发送时，需要发送service_name,method_name，方法参数大小，方法参数       定义proto的message类型，进行数据头的序列化和反序列化
                                                       数据头：除了方法参数以外的前面所有的数据
                                                       防止Tcp通信数据粘包，需要提供方法参数的长度
header_size(4个字节)+header_str+arg_atr
header_size:service_name,method_name,agrs_size的长度
*/
// 如果远程有一个rpc服务调用请求，Onmessage方法就会响应
void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer *buf, muduo::Timestamp time)
{ // 网络接受的远程rpc调用请求的字符流   Login  args(参数)
    std::string recv_buf = buf->retrieveAllAsString();
    // 从字符流中读取前四个字节的内容
    uint32_t header_size = 0;
    recv_buf.copy((char *)&header_size, 4, 0); // 从起始位置读4个字节的内容放在header_size,按二进制存储
    // 读取header_size读取原始字符流,反序列化数据
    std::string rpc_header_str = recv_buf.substr(4, header_size);
    mprpc::RpcHeader rpcheader;
    std::string service_name;
    std::string method_name;
    uint32_t args_size;
    if (rpcheader.ParseFromString(rpc_header_str))
    {
        // 数据头反序列化成功
        service_name = rpcheader.service_name();
        method_name = rpcheader.method_name();
        args_size = rpcheader.args_size();
    }
    else
    {
        // 数据头反序列化失败
        LOG_ERR("rpc_head_str %s parse error!",rpc_header_str.c_str());
        return;
    }
    // 获取rpc方法参数的字符流数据
    std::string args_atr = recv_buf.substr(4 + header_size, args_size);
    // 打印调试的信息
    std::cout << "=============================================" << std::endl;
    std::cout << "header_size: " << header_size << std::endl;
    std::cout << "rpc_header_str: " << rpc_header_str << std::endl;
    std::cout << "header_size: " << header_size << std::endl;
    std::cout << "service_name: " << service_name << std::endl;
    std::cout << "method_name: " << method_name << std::endl;
    std::cout << "args_size: " << args_size << std::endl;
    std::cout << "args_atr: " << args_atr << std::endl;
    std::cout << "=============================================" << std::endl;
    // 获取service对象和method对象
    auto it = m_serviceMap.find(service_name);
    if (it == m_serviceMap.end())
    {
        std::cout << service_name << " is not exist!" << std::endl;
        return;
    }

    auto mit = it->second.m_methodMap.find(method_name);
    if (mit == it->second.m_methodMap.end())
    {
        std::cout << method_name << " is not exist!" << std::endl;
        return;
    }
    google::protobuf::Service *service = it->second.m_service;      // 获取service对象
    const google::protobuf::MethodDescriptor *method = mit->second; // 获取method对象
    // 生成rpc方法调用request和response参数   其实就是callee和caller两端的proto规则
    google::protobuf::Message* request = service->GetRequestPrototype(method).New();
    google::protobuf::Message* response = service->GetResponsePrototype(method).New();
    if (!request->ParseFromString(args_atr))  //反序列化的结果放入request中，而不是字符串上
    {   
        LOG_ERR("request parse error,content: %s",args_atr.c_str());
        return;
    }
    // 给下面的method方法的调用，绑定一个closure的回调函数
    google::protobuf::Closure*  done=google::protobuf::NewCallback<RpcProvider,const muduo::net::TcpConnectionPtr&,google::protobuf::Message*>(this,&RpcProvider::SendRpcResponse,conn,response);
    // 在框架上根据远端rpc请求，调用当前rpc节点上发布的方法
    // 相当于UserService->CallMethod()，CallMethod()中会通过方法的索引进行函数调用。又因为UserService是子类因此，所调用的函数在UserService被重写
    service->CallMethod(method, nullptr, request, response, done);// 这个就是被调用端重写的那个函数
}

// closure回调操作，用于序列化rpc的响应和网络发送
void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr& conn, google::protobuf::Message* response)
{
    std::string response_str;
    if(response->SerializePartialToString(&response_str)){
       //序列化后，通过网络把rpc方法执行的结果发送给rpc调用方
        conn->send(response_str);
    }else{
        LOG_ERR("serialize response_str error!");

    }
    conn->shutdown();//模拟http的短链接服务，由rpcprovider主动断开连接
}