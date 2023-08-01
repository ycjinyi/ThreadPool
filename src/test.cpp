#include "threadpool.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <memory>

class MyTask: public Task {
public:
    void run() override {
        std::cout << std::this_thread::get_id() << std::endl;
    }
};

int main() {
    ThreadPool threadPool;
    threadPool.start(2);
    threadPool.submitTask(std::make_shared<MyTask>());
    threadPool.submitTask(std::make_shared<MyTask>());
    threadPool.submitTask(std::make_shared<MyTask>());
    threadPool.submitTask(std::make_shared<MyTask>());
    threadPool.submitTask(std::make_shared<MyTask>());
    std::this_thread::sleep_for(std::chrono::seconds(5));
    return 0;
}