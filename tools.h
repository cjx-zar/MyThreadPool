#pragma once
#include <functional>

namespace tools{
    class Task{
    public:
        Task(std::function<void()> f): handle(f) {}
        Task(){}
        void call(){
            handle();
        }
        Task(Task&) = delete;
        Task(const Task&) = delete;
        Task& operator = (const Task&) = delete;

        Task(Task&& t):handle(t.handle){
            t.handle = nullptr;
        }

        Task& operator = (Task&& t){
            handle = t.handle;
            t.handle = nullptr;
            return *this;
        }
        bool working(){
            return !!handle;
        }
    private:
        std::function<void()> handle = nullptr;
    };
    
}