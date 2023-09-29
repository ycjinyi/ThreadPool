#include "threadpool.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <memory>

class MyTask: public Task {
public:
    Any run() override {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        return 100;
    }
};

int main() {
    ThreadPool threadPool;
    threadPool.start(2, PoolMode::CACHED_MODE, 5);
    Result res1 = threadPool.submitTask(std::make_shared<MyTask> ());
    Result res2 = threadPool.submitTask(std::make_shared<MyTask> ());
    threadPool.submitTask(std::make_shared<MyTask> ());
    threadPool.submitTask(std::make_shared<MyTask> ());
    threadPool.submitTask(std::make_shared<MyTask> ());
    threadPool.submitTask(std::make_shared<MyTask> ());
    //std::cout << res1.get().cast<int>() + res2.get().cast<int>() << std::endl;
    getchar();
    return 0;
}