#pragma once
#include <semaphore.h>
#include <zookeeper/zookeeper.h>
#include <string>
/*
主要是客户端一个注册中心
*/
//封装一个zk客户端类
class ZkClient
{
public:
    ZkClient();
    ~ZkClient();

    //zkclient启动连接zkserver
    void Start();
    //在zkserver上根据指定的path创建znode节点
    void Create(const char *path, const char *data, int detalen, int state = 0);
    //根据参数指定的znode节点路径，获取znode节点的值
    std::string GetDate(const char *path);

private:
    //zk的客户端句柄，通过它操作zookeeper
    zhandle_t* m_zhandle;
};