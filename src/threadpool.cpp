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
    maxThreadNum_(0), isStarted_(false),
    taskQueMaxNum_(TASK_QUE_MAX_NUM), poolMode_(PoolMode::FIX_MODE) {}
//析构函数
ThreadPool::~ThreadPool() {}
//给线程池添加任务, 用户调用，作为生产者，返回执行结果Result
Result ThreadPool::submitTask(std::shared_ptr<Task> task) {
    std::unique_lock<std::mutex> ulock(taskQueMutex_);
    //当lambda表达式为真, 就向下执行. 否则就阻塞在taskQueNotFull_条件变量上1s
    //如果1s后还是没有满足lambda表达式, 则返回false
    if(!taskQueNotFull_.wait_for(ulock, std::chrono::seconds(1), 
        [&]() -> bool { return taskQueNum_ < taskQueMaxNum_; })) {
        return Result(task, false);
    };
    taskQue_.push(task);
    ++taskQueNum_;
    taskQueNotEmpty_.notify_all();
    return Result(task);
}
//定义线程函数, 线程执行, 作为消费者
void ThreadPool::threadFunc() {
    while(true) {
        std::shared_ptr<Task> task(nullptr);
        {
            std::unique_lock<std::mutex> ulock(taskQueMutex_);
            //满足条件就继续执行, 否则一直阻塞在taskQueNotEmpty_条件变量上
            taskQueNotEmpty_.wait(ulock, [&]() -> bool { return taskQueNum_ > 0;});
            task = taskQue_.front();
            taskQue_.pop();
            --taskQueNum_;
            if(taskQueNum_) taskQueNotEmpty_.notify_all();
            taskQueNotFull_.notify_all();
        }
        if(task != nullptr) task->execute();
    }
}
//运行
void ThreadPool::start(uint initThreadNum, PoolMode poolMode, uint maxThreadNum) {
    initThreadNum_ = initThreadNum;
    poolMode_ = poolMode;
    maxThreadNum_ = poolMode_ == PoolMode::FIX_MODE ? 
        initThreadNum: std::max(initThreadNum, maxThreadNum);
    //创建线程
    for(int i = 0; i < initThreadNum; ++i) {
        std::unique_ptr<Thread> ptr = 
            std::make_unique<Thread> (std::bind(&ThreadPool::threadFunc, this));
        threads_.emplace_back(std::move(ptr));
    }
    //启动线程
    for(auto& thread: threads_) thread->run();
    isStarted_ = true;
} 

void ThreadPool::start(uint initThreaNum, PoolMode poolMode) {
    return start(initThreaNum, poolMode, initThreaNum);
}

void ThreadPool::start(uint initThreadNum) {
    return start(initThreadNum, PoolMode::FIX_MODE, initThreadNum);
}

//-------------------------------Task------------------------------
void Task::execute() {
    if(result_ == nullptr) return;
    // 执行任务并设置返回值
    result_->setValue(run());
}
void Task::setResult(Result* result) {
    result_ = result;
}