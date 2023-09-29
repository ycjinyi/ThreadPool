#include "threadpool.hpp"
#include <iostream>
const uint TASK_QUE_MAX_NUM = 1024;
const uint MAX_THREAD_IDLE_TIME_S = 10;

//-------------------------------Thread------------------------------
//创建线程对象，指定线程工作函数
Thread::Thread(ThreadFunc threadFunc): threadFunc_(threadFunc) {}
//创建实际的线程并执行指定的线程工作函数
void Thread::run(uint threadNumber) {
    std::thread t(threadFunc_, threadNumber);
    //设置线程分离后，由于主线程不再具有管理子线程的能力，此时的t.get_id()将无效
    std::cout << "creat thread: " << t.get_id() << std::endl;
    t.detach();//线程分离
}

//-------------------------------ThreadPool------------------------------
//构造函数
ThreadPool::ThreadPool(): initThreadNum_(0), maxThreadNum_(0), 
    poolMode_(PoolMode::FIX_MODE), isStarted_(false), idleThreadNum_(0), 
    nowThreadNum_(0), taskNum_(0), taskQueMaxNum_(TASK_QUE_MAX_NUM),
    threadNumber_(0) {}
//析构函数
ThreadPool::~ThreadPool() {}
//给线程池添加任务, 用户调用，作为生产者，返回执行结果Result
Result ThreadPool::submitTask(std::shared_ptr<Task> task) {
    std::unique_lock<std::mutex> ulock(taskQueMutex_);
    //当lambda表达式为真, 就向下执行. 否则就阻塞在taskQueNotFull_条件变量上1s
    //如果1s后还是没有满足lambda表达式, 则返回false
    if(!taskQueNotFull_.wait_for(ulock, std::chrono::seconds(1), 
        [&]() -> bool { return taskNum_ < taskQueMaxNum_; })) {
        return Result(task, false);
    };
    taskQue_.push(task);
    ++taskNum_;
    taskQueNotEmpty_.notify_all();
    //在cached模式下，如果空闲线程数量小于任务数量且小于最大线程数量，则会创建新线程
    if(poolMode_ == PoolMode::CACHED_MODE 
        && idleThreadNum_ < taskNum_
        && nowThreadNum_ < maxThreadNum_) {
        creatThread();
    }
    return Result(task);
}
//定义线程函数, 线程执行, 作为消费者
void ThreadPool::threadFunc(uint threadNumber) {
    auto lastTime = std::chrono::high_resolution_clock::now();
    while(true) {
        std::shared_ptr<Task> task(nullptr);
        {
            std::unique_lock<std::mutex> ulock(taskQueMutex_);
            //如果是cached模式，当线程总数大于初始线程数时，将回收空闲时间超过60s的线程
            if(poolMode_ == PoolMode::CACHED_MODE && nowThreadNum_ > initThreadNum_) {
                while(taskNum_ <= 0) {
                    if(std::cv_status::timeout ==
                        taskQueNotEmpty_.wait_for(ulock, std::chrono::seconds(1))) {
                        auto nowTime = std::chrono::high_resolution_clock::now();
                        auto duration = std::chrono::duration_cast<std::chrono::seconds> (nowTime - lastTime);
                        //如果超时，将回收当前线程，并减少计数和线程map中保存的线程对象
                        if(duration.count() > MAX_THREAD_IDLE_TIME_S 
                            && nowThreadNum_ > initThreadNum_) {
                            --idleThreadNum_;
                            --nowThreadNum_;
                            threads_.erase(threadNumber);
                            std::cout << "remove thread: "
                                << std::this_thread::get_id() << std::endl;
                            return;
                        }
                    }
                }
            } else {
                //满足条件就继续执行, 否则一直阻塞在taskQueNotEmpty_条件变量上
                taskQueNotEmpty_.wait(ulock, [&]() -> bool { return taskNum_ > 0;});
            }
            std::cout << "thread: " << std::this_thread::get_id() 
                << " get task!" << std::endl;
            --idleThreadNum_;
            task = taskQue_.front();
            taskQue_.pop();
            --taskNum_;
            if(taskNum_) taskQueNotEmpty_.notify_all();
            taskQueNotFull_.notify_all();
        }
        if(task != nullptr) task->execute();
        ++idleThreadNum_;
        lastTime = std::chrono::high_resolution_clock::now();
    }
}
//运行
void ThreadPool::start(uint initThreadNum, PoolMode poolMode, uint maxThreadNum) {
    initThreadNum_ = initThreadNum;
    poolMode_ = poolMode;
    maxThreadNum_ = poolMode_ == PoolMode::FIX_MODE ? 
        initThreadNum: std::max(initThreadNum, maxThreadNum);
    //创建并启动线程
    for(int i = 0; i < initThreadNum; ++i) {
        creatThread();
    }
    isStarted_ = true;
} 

void ThreadPool::start(uint initThreaNum, PoolMode poolMode) {
    return start(initThreaNum, poolMode, initThreaNum);
}

void ThreadPool::start(uint initThreadNum) {
    return start(initThreadNum, PoolMode::FIX_MODE, initThreadNum);
}

uint ThreadPool::getThreadNumber() {
    while(usedThreadNumbers_.find(threadNumber_) != usedThreadNumbers_.end()) {
        ++threadNumber_;
    }
    usedThreadNumbers_.insert(threadNumber_);
    return threadNumber_++;
}

void ThreadPool::creatThread() {
    uint threadNumber = getThreadNumber();
    std::unique_ptr<Thread> ptr = 
        std::make_unique<Thread> (std::bind(&ThreadPool::threadFunc, this, std::placeholders::_1)); 
    ++nowThreadNum_;
    ++idleThreadNum_;
    ptr->run(threadNumber);    
    threads_.emplace(threadNumber, std::move(ptr));
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