#include "result.hpp"
#include "threadpool.hpp"

Result::Result(std::shared_ptr<Task> task, bool valid): task_(task), valid_(valid) {
    // 反向设置task中的result，便于线程函数执行后设置结果
    task_->setResult(this);
}

// 获取任务运行结果，用户调用
Any Result::get() {
    if(!valid_) {
        return "";
    }
    semaphore_.wait();
    return std::move(res_);
}

// 设置任务的运行结果，线程运行结束调用
void Result::setValue(Any res) {
    res_ = std::move(res);
    semaphore_.post();
}
