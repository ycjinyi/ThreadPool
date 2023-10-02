#include "threadpool.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <memory>

// class MyTask: public Task {
// public:
//     Any run() override {
//         std::this_thread::sleep_for(std::chrono::seconds(1));
//         return 100;
//     }
// };

int sum(int a, int b) {
    std::this_thread::sleep_for(std::chrono::seconds(200));
    return a + b;
}

int main() {
    // {
    //     ThreadPool threadPool;
    //     threadPool.start(2, PoolMode::CACHED_MODE, 10);
    //     Result res1 = threadPool.submitTask(std::make_shared<MyTask> ());
    //     Result res2 = threadPool.submitTask(std::make_shared<MyTask> ());
    //     threadPool.submitTask(std::make_shared<MyTask> ());
    //     // threadPool.submitTask(std::make_shared<MyTask> ());
    //     // threadPool.submitTask(std::make_shared<MyTask> ());
    //     // threadPool.submitTask(std::make_shared<MyTask> ());
    //     //std::cout << res1.get().cast<int>() + res2.get().cast<int>() << std::endl;
    //     std::this_thread::sleep_for(std::chrono::seconds(1));
    // }
    // std::cout << "main over!" << std::endl;
    {
        ThreadPool threadPool;
        threadPool.start(2, PoolMode::CACHED_MODE, 3);
        std::future<int> res1 = threadPool.submitTask(sum, 1, 3);
        //std::cout << res1.get() << std::endl;
        threadPool.submitTask(sum, 2, 4);
        threadPool.submitTask(sum, 2, 4);
        threadPool.submitTask([] (int c) { return c; }, 2);
        threadPool.submitTask([] (int a, int b, int c) { return a + b - c;}, 2, 4, 5);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    getchar();
    return 0;
}