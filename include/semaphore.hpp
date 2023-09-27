#ifndef SEMAPHORE_H
#define SEMAPHORE_H
#include <condition_variable>
#include <mutex>
//实现信号量
class Semaphore {
public:
    Semaphore(int limit = 0): limit_(limit) {}
    //wait函数，用于消耗信号量，当没有时阻塞
    void wait() {
        std::unique_lock<std::mutex> ulock(mutex_);
        cond_.wait(ulock, [&]() -> bool { return limit_ > 0; });
        --limit_;
    }
    //post函数，用于释放信号量
    void post() {
        std::unique_lock<std::mutex> ulock(mutex_);
        ++limit_;
        cond_.notify_all();
    }
private:
    int limit_;
    std::condition_variable cond_;
    std::mutex mutex_;
};
#endif