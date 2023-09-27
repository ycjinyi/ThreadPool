#include "semaphore.hpp"

Semaphore::Semaphore(int limit): limit_(limit) {}
//wait函数，用于消耗信号量，当没有时阻塞
void Semaphore::wait() {
    std::unique_lock<std::mutex> ulock(mutex_);
    cond_.wait(ulock, [&]() -> bool { return limit_ > 0; });
    --limit_;
}
//post函数，用于释放信号量
void Semaphore::post() {
    std::unique_lock<std::mutex> ulock(mutex_);
    ++limit_;
    cond_.notify_all();
}