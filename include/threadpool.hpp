#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#include <vector>
#include <queue>
#include <memory>
#include <mutex>
#include <atomic>
#include <functional>
#include <condition_variable>
#include <thread>

#include "any.hpp"

//线程池的工作模式
enum class PoolMode {
    FIX_MODE, CACHED_MODE
};

//任务的抽象基类
class Task {
public:
    virtual Any run() = 0;
};

//线程类
class Thread {
public:
    using ThreadFunc = std::function<void ()>;
    Thread(ThreadFunc threadFunc);
    void run();
private:
    ThreadFunc threadFunc_;
};

//线程池类
class ThreadPool {
public:
    ThreadPool();
    ~ThreadPool();
    //删除拷贝构造
    ThreadPool(const ThreadPool&) = delete;
    //删除拷贝赋值
    ThreadPool& operator= (const ThreadPool&) = delete;
    //设置工作模式
    void setPoolMode(PoolMode poolMode);
    //设置任务队列中任务的最大数
    void setTaskQueMaxNum(uint taskQueNum);
    //给线程池添加任务
    void submitTask(std::shared_ptr<Task> task);
    //运行
    void start(uint16_t threadNum);
private:
    //定义线程函数
    void threadFunc();
private:
    //线程列表
    std::vector<std::unique_ptr<Thread>> threads_; 
    //初始线程数量
    uint16_t initThreadNum_;
    //任务队列
    std::queue<std::shared_ptr<Task>> taskQue_;
    //当前任务数量
    std::atomic_uint taskQueNum_;
    //任务的最大上限数
    uint taskQueMaxNum_;
    //线程池的工作模式
    PoolMode poolMode_;
    
    //互斥锁，用于保障任务队列的线程安全
    std::mutex taskQueMutex_;
    //条件变量，用于生产者和消费者线程之间的同步
    std::condition_variable taskQueNotFull_;
    std::condition_variable taskQueNotEmpty_;
};
#endif