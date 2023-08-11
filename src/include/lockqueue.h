#pragma once
#include <mutex>
#include <thread>
#include <queue>
#include <condition_variable>
// 异步写日志队列
template <typename T> // 设置一个队列模板(存储队列的数据可以在创建对象时任意设置)
class LockQueue
{
public:
    //多个线程都会写日志queue
    void push(const T &data){
        std::lock_guard<std::mutex> lock(m_mutex); //智能指针可以加锁和释放锁
        m_queue.push(data); 
        m_condition.notify_one();
    }
    //一个线程都会读日志queue
    T pop(){
        std::unique_lock<std::mutex> lock(m_mutex);//wait需要这样的lock锁
        while(m_queue.empty()){
            //日志队列为空，线程进入wait状态
            m_condition.wait(lock);
        }
        T data=m_queue.front();
        m_queue.pop();
        return data;
    }

private:
    std::queue<T> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_condition;
};