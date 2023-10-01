#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#include <unordered_map>
#include <queue>
#include <memory>
#include <mutex>
#include <atomic>
#include <functional>
#include <condition_variable>
#include <thread>
#include <unordered_set>
#include <future>

#define USE_FUTURE 1

#if !USE_FUTURE
#include "any.hpp"
#include "result.hpp"
#endif

#if !USE_FUTURE
//任务的抽象基类
class Task {
public:
    Task();
    void execute();
    void setResult(Result*);
    void setValid(bool);
    std::mutex& getMutex();
    virtual Any run() = 0;
private:
    bool valid_;
    Result* result_;
    std::mutex mutex_;
};
#endif

//线程池的工作模式
enum class PoolMode {
    FIX_MODE, CACHED_MODE
};

//线程类
class Thread {
public:
    using ThreadFunc = std::function<void (uint)>;
    Thread(ThreadFunc threadFunc);
    void run(uint threadNumber); 
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

    //给线程池添加任务
    #if !USE_FUTURE
    Result submitTask(std::shared_ptr<Task> task);
    #else
    //采用可变参数模板
    template<typename Func, typename ...Args>
    auto submitTask(Func&& f, Args&& ...args) -> std::future<decltype(f(args...))> {
        using RET_TYPE = decltype(f(args...));

        std::unique_lock<std::mutex> ulock(taskQueMutex_);
        if(!taskQueNotFull_.wait_for(ulock, std::chrono::seconds(1), 
            [&]() -> bool { return taskNum_ < taskQueMaxNum_; })) {
            //如果添加失败，就返回空
            auto task = std::make_shared<std::packaged_task<RET_TYPE()>> 
                ([]() -> RET_TYPE { return RET_TYPE();});
            std::future<RET_TYPE> res = task->get_future();
            (*task)();
            return res;
        };

        //注意传入其它函数的所有使用了万能引用的参数，均需要完美转发
        auto task = std::make_shared<std::packaged_task<RET_TYPE()>>
            (std::bind(std::forward<Func>(f), std::forward<Args>(args)...));
        std::future<RET_TYPE> res = task->get_future();
        taskQue_.push([task]() { (*task)(); });
        ++taskNum_;
        taskQueNotEmpty_.notify_all();
        if(poolMode_ == PoolMode::CACHED_MODE 
            && idleThreadNum_ < taskNum_
            && nowThreadNum_ < maxThreadNum_) {
            creatThread();
        }
        return res;
    }
    #endif
    
    //运行
    void start(uint initThreadNum, PoolMode poolMode, uint maxThreadNum_);
    void start(uint initThreaNum, PoolMode poolMode);
    void start(uint initThreadNum = std::thread::hardware_concurrency());
    //获得线程编号
    uint getThreadNumber();
private:
    //定义线程函数
    void threadFunc(uint);
    //创建线程
    void creatThread();
    //移除线程对象
    void removeThread(uint);
private:
    //线程列表
    std::unordered_map<uint, std::unique_ptr<Thread>> threads_; 
    //初始线程数量
    uint initThreadNum_;
    //线程数量的上限值
    uint maxThreadNum_;
    //线程池的工作模式
    PoolMode poolMode_;
    //当前线程池是否已经启动
    std::atomic_bool isStarted_;
    //当前空闲的线程数目
    std::atomic_uint idleThreadNum_;
    //当前线程的数目
    std::atomic_uint nowThreadNum_;
    //销毁线程时用于线程数目的锁
    std::mutex threadNumberMutex_;

    #if !USE_FUTURE
    //任务队列
    std::queue<std::shared_ptr<Task>> taskQue_;
    #else
    using Task = std::function<void ()>; 
    std::queue<Task> taskQue_;
    #endif
    //当前任务数量
    std::atomic_uint taskNum_;
    //任务的最大上限数
    uint taskQueMaxNum_;
    
    //互斥锁，用于保障任务队列的线程安全
    std::mutex taskQueMutex_;
    //条件变量，用于生产者和消费者线程之间的同步
    std::condition_variable taskQueNotFull_;
    std::condition_variable taskQueNotEmpty_;
    //用于销毁线程时的唤醒
    std::condition_variable removeThread_;
    
    //线程编号
    uint threadNumber_;
    std::unordered_set<uint> usedThreadNumbers_;
};
#endif