#include "./include/logger.h"
#include <iostream>
#include <string>
#include <time.h>
// 获取日志单例
Logger &Logger::GetInstance()
{
    static Logger m_logger;
    return m_logger;
}
Logger::Logger()
{
    // 启动专门线程写日志
    std::thread writeLogTask([&]()
                             {
        for(;;){
            //获取当前日期，然后取日志信息，写入相应的日志文件当中  a+:当文件名不存在，创建文件
            time_t now=time(nullptr);
            tm* nowtm=localtime(&now);
            char file_name[128];
            sprintf(file_name,"../log/%d-%d-%d-log.txt",nowtm->tm_year+1990,nowtm->tm_mon+1,nowtm->tm_mday);
            FILE * pf=fopen(file_name,"a+");
            if(pf==nullptr){
                std::cout<<"logger file: "<<file_name<<" open error!"<<std::endl;
                exit(EXIT_FAILURE);
            }
            std::string msg=m_lockqueue.pop();
            char timebuf[128];
            sprintf(timebuf,"%d:%d:%d=>[%s]",nowtm->tm_hour,nowtm->tm_min,nowtm->tm_sec,(m_loglevel==INFO?"info":"error"));//日志信息加入时分秒
            msg.insert(0,timebuf);
            msg.append("\n");
            fputs(msg.c_str(),pf);//向文件中写日志
            fclose(pf);
            
            } });
    // 设置分离线程
    writeLogTask.detach();
}
// 设置日志级别
void Logger::SetLogLevel(LogLeve1 level)
{
    m_loglevel = level;
}
// 写日志,把日志信息写入lockqueue队列缓冲区中
void Logger::Log(std::string msg)
{
    m_lockqueue.push(msg);
}