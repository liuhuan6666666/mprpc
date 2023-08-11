#pragma once
#include "lockqueue.h"
enum LogLeve1
{
    INFO,  // 普通信息
    ERROR, // 错误信息
};
// Mprpc框架提供的日志系统
class Logger
{
public:
    //获取日志单例
    static Logger& GetInstance();
    // 设置日志级别
    void SetLogLevel(LogLeve1 level);
    // 写日志,把日志信息写入lockqueue队列缓冲区中
    void Log(std::string msg);

private:
    int m_loglevel; // 记录日志级别
    LockQueue<std::string> m_lockqueue;
    Logger();
    Logger(const Logger &) = delete;
    Logger(Logger &&) = delete;
};

//定义宏   用户直接用 LOG_INFO 和 LOG_ERR，不用再去调用函数
#define LOG_INFO(logmsgformat,...)\
    do\
    {\
        Logger& logger=Logger::GetInstance();\
        logger.SetLogLevel(INFO);\
        char c[1024];\
        snprintf(c,1024,logmsgformat, ##__VA_ARGS__);\
        logger.Log(c);\
    } while(0);

#define LOG_ERR(logmsgformat,...)\
    do\
    {\
        Logger& logger=Logger::GetInstance();\
        logger.SetLogLevel(ERROR);\
        char c[1024];\
        snprintf(c,1024,logmsgformat,##__VA_ARGS__);\
        logger.Log(c);\
    } while(0);