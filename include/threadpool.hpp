#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#include <vector>
#include <queue>
#include <memory>
#include <mutex>
#include <atomic>
#include <condition_variable>

//线程池的工作模式
enum class PoolMode {
    FIX_MODE, CACHED_MODE
};

//任务的抽象基类
class Task {
public:
    virtual void run() = 0;
};

//线程类
class Thread {
public:
private:
};

//线程池类
class ThreadPool {
public:
    ThreadPool();
    ~ThreadPool();
    //设置工作模式
    void setPoolMode(PoolMode poolMode);
    //运行
    void start();
private:
    //线程列表
    std::vector<Thread*> threads_; 
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