#include "./include/zookeeperutil.h"
#include"mprpcapplication.h"
#include<string>
//全局回调wather函数   zk给zkclient发通知
void global_watcher(zhandle_t *zh,int type,int state,const char *path,void *watchweCtx){
    if(type==ZOO_SESSION_EVENT){  //回调消息类型是和会话相关的消息类型
        if(state==ZOO_CONNECTED_STATE){  //zkclient与zkserver连接成功
            sem_t *sem=(sem_t*)zoo_get_context(zh);  //从句柄上拿信号量
            sem_post(sem); //信号量资源加1
        }
    }
}
ZkClient::ZkClient() :m_zhandle(nullptr)
{
}
ZkClient::~ZkClient()
{
    if(m_zhandle!=nullptr){
        zookeeper_close(m_zhandle);//关闭句柄，释放资源 MySQL_Conn
    }
}

// zkclient启动连接zkserver
void ZkClient::Start()
{
    std::string host=MprpcApplication::GetInstance().Getconfig().Load("zookeeperip");
    std::string port=MprpcApplication::GetInstance().Getconfig().Load("zookeeperport");
    std::string connstr=host+":"+port;
    /*
    zookeeper_mt：多线程版本
    zookeeper的API客户端提供了三个线程：
                                    API调用线程，
                                    网络I/O线程， pthread_create  底层使用poll技术
                                    watcher回调线程    zkserver给zkclient发送响应
    */
   //连接zkserver
    m_zhandle=zookeeper_init(connstr.c_str(),global_watcher,30000,nullptr,nullptr,0);  //连接是异步，直到state==ZOO_CONNECTED_STATE才算连接成功
    if(m_zhandle==nullptr){  //只能看句柄开辟是否成功，有可能内存不够用
        std::cout<<"zookeeper_init error!"<<std::endl;
        exit(EXIT_FAILURE);
    }
    sem_t sem;//信号量
    sem_init(&sem,0,0);
    zoo_set_context(m_zhandle,&sem); //为句柄设置信号量
    sem_wait(&sem);
    std::cout<<"zookeeper_init success!"<<std::endl;
}
// 在zkserver上根据指定的path创建znode节点
void ZkClient::Create(const char *path, const char *data, int datalen, int state)
{
    char path_buffer[128];
    int bufferlen=sizeof(path_buffer);
    int flag;
    flag=zoo_exists(m_zhandle,path,0,nullptr);//判断节点是否存在
    if(ZNONODE==flag){  
        flag=zoo_create(m_zhandle,path,data,datalen,&ZOO_OPEN_ACL_UNSAFE,state,path_buffer,bufferlen); //state=ZOO_EPHEMERAL：临时性节点   state=ZOO_SEQUENCE:永久性节点
        if(flag==ZOK){  //ZOK:运行成功
            std::cout<<"znode create success... path: "<<path<<std::endl;
        }else{
            std::cout<<"flag: "<<flag<<std::endl;
            std::cout<<"znode create error... path: "<<path<<std::endl;
            exit(EXIT_FAILURE);
        }
    }
}

// 根据参数指定的znode节点路径，获取znode节点的值
std::string ZkClient::GetDate(const char *path)
{
    char buffer[64];
    int bufferlen=sizeof(buffer);
    int flag=zoo_get(m_zhandle,path,0,buffer,&bufferlen,nullptr);
    if(flag!=ZOK){
        std::cout<<"get znode error... path: "<<path<<std::endl;
        return "";
    }else{
        return buffer;
    }
}