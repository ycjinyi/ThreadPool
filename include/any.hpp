#ifndef ANY_H
#define ANY_H
#include <memory>
//any类，用于接受任意的参数
class Any {
public:
    Any() = default;
    Any(const Any&) = delete;
    Any& operator= (const Any&) = delete;
    Any(Any&&) = default;
    Any& operator= (Any&&) = default;
    //利用模板作为构造函数的参数，用于接收任意的数据类型
    template<typename T>
    Any(T data): base_(std::make_unique<Derive<T>> (data)) {}
    //将Any类转为目标数据
    template<typename T>
    T cast() {
        Derive<T>* dp = dynamic_cast<Derive<T>*> (base_.get());
        if(dp == nullptr) {
            throw "type is incompatible";
        }
        return dp->data_;
    }
private:
    class Base {
    public:
        virtual ~Base() = default;
    };
    template<typename T>
    class Derive: public Base {
    public:
        Derive(T data): data_(data) {}
        T data_;
    };
private:
    std::unique_ptr<Any::Base> base_;
};
#endif