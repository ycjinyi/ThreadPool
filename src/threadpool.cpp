#include "threadpool.hpp"
#include <iostream>
const uint TASK_QUE_MAX_NUM = 1024;

//-------------------------------Thread------------------------------
//创建线程对象，指定线程工作函数
Thread::Thread(ThreadFunc threadFunc): threadFunc_(threadFunc) {}
//创建实际的线程并执行指定的线程工作函数
void Thread::run() {
    std::thread t(threadFunc_);
    t.detach();//线程分离
}

//-------------------------------ThreadPool------------------------------
//构造函数
ThreadPool::ThreadPool(): initThreadNum_(0), taskQueNum_(0), 
    taskQueMaxNum_(TASK_QUE_MAX_NUM), poolMode_(PoolMode::FIX_MODE) {}
//析构函数
ThreadPool::~ThreadPool() {}
//设置工作模式
void ThreadPool::setPoolMode(PoolMode poolMode) {
    poolMode_ = poolMode;
}
//设置任务队列中任务的最大数
void ThreadPool::setTaskQueMaxNum(uint taskQueNum) {
    taskQueMaxNum_ = taskQueNum;
}
//给线程池添加任务, 用户调用，作为生产者
void ThreadPool::submitTask(std::shared_ptr<Task> task) {
    std::unique_lock<std::mutex> ulock(taskQueMutex_);
    while(taskQueNum_ == taskQueMaxNum_) {
        taskQueNotFull_.wait(ulock);
    }
    taskQue_.push(task);
    ++taskQueNum_;
    taskQueNotEmpty_.notify_all();
}
//定义线程函数, 线程执行, 作为消费者
void ThreadPool::threadFunc() {
    while(true) {
        std::shared_ptr<Task> task(nullptr);
        {
            std::unique_lock<std::mutex> ulock(taskQueMutex_);
            while(taskQueNum_ == 0) {
                taskQueNotEmpty_.wait(ulock);
            }
            task = taskQue_.front();
            taskQue_.pop();
            --taskQueNum_;
        }
        taskQueNotFull_.notify_all();
        if(task) task->run();
     }
}
//运行
void ThreadPool::start(uint16_t threadNum) {
    initThreadNum_ = threadNum;
    //创建线程
    for(int i = 0; i < initThreadNum_; ++i) {
        std::unique_ptr<Task> ptr = 
            std::make_unique<Task> (std::bind(&ThreadPool::threadFunc, this));
        threads_.emplace_back(std::move(ptr));
    }
    //启动线程
    for(auto& thread: threads_) thread->run();
}