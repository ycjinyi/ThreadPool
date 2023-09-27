#ifndef RESULT_H
#define RESULT_H
#include <memory>
#include "semaphore.hpp"
#include "any.hpp"

class Task;

class Result {
public:
    Result(std::shared_ptr<Task> task, bool valid = true);
    // 获取任务运行结果，用户调用
    Any get();
    // 设置任务的运行结果，线程运行结束调用
    void setValue(Any res);
private:
    Any res_;
    bool valid_;
    // 和result关联的task
    std::shared_ptr<Task> task_;
    Semaphore semaphore_;
};
#endif