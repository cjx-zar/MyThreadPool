#pragma once
#include <functional>
#include <chrono>

namespace tools{
    template <typename F, typename... _Args>
    double timewait(F&& foo, _Args&&... argv) {
        auto time_start = std::chrono::steady_clock::now();
        foo(std::forward<_Args>(argv)...);
        auto time_end = std::chrono::steady_clock::now();
        return std::chrono::duration<double>(time_end-time_start).count();
    }


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