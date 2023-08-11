#pragma once
#include <google/protobuf/service.h>
#include <string>
//为了防止在response没有建立前就访问，添加controller控制信息

class MprpcController : public google::protobuf::RpcController
{
public:
    MprpcController();
    void Reset();
    bool Failed() const;
    std::string ErrorText() const;
    void SetFailed(const std::string &reason);

    // 目前为实现功能
    void StartCancel();
    bool IsCanceled() const;
    void NotifyOnCancel(google::protobuf::Closure *callback);
private:
    bool m_failed;//rpc方法执行过程中的状态
    std::string m_errText;//RPC方法执行过程中的错误信息
};