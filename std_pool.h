#pragma once
#include "./header.h"

// 按照JAVA中的线程池特性，实现C++版本的标准线程池(std_pool)
// 
// 核心组件：核心线程数、最大线程数、阻塞队列、拒绝策略
// 1.如果线程数小于核心线程数，那么添加工作线程处理当前任务（初始阶段）
// 2.如果线程数大于等于核心线程数:
//      先判断阻塞队列是否满，如果没满，那么将任务添加到阻塞队列
//      如果满了，那么判断核心数是否小于最大线程数，如果小于，那么添加工作线程
//      如果仍不满足，那么执行设置好的拒绝策略

namespace threadpool{
    class Std_pool{
    public:
        Std_pool(const std::string& str, int max_cap=INT_MAX): max_task_cap(max_cap){
            int cpus = sysconf(_SC_NPROCESSORS_CONF);
            std::cout << "Total cores : " << cpus << std::endl;
            if(str == "IO") {
                core_thd_num = 2 * cpus;
                max_thd_num = 25 * cpus;
            }else if(str == "CPU"){
                core_thd_num = cpus;
                max_thd_num = 2 * cpus;
            }else{
                std::cout << "Illegal configuration, execute the default constructor" << std::endl;
                core_thd_num = 10;
                max_thd_num = 1000;
            };
        }
        Std_pool(int core_num=10, int max_num=1000, int max_cap=INT_MAX):
        core_thd_num(core_num), max_thd_num(max_num), max_task_cap(max_cap){}

        ~Std_pool(){
            if(!stop){
                close();
            }
        }

        template <typename _Callable, typename... Args>
        auto submit(_Callable&& f, Args&&... args) -> std::future<decltype(f(std::forward<Args>(args)...))>{
            using RT = decltype(f(std::forward<Args>(args)...));

            std::function<RT()> func = std::bind(std::forward<_Callable>(f), std::forward<Args>(args)...);
            auto func_ptr = std::make_shared<std::packaged_task<RT()>>(func);

            std::function<void()> warpper_func_ptr = [func_ptr](){
                (*func_ptr)();
            };

            {
                lock_guard guard(m_lock);
                Task task(warpper_func_ptr);

                if(cur_thd_num < core_thd_num){
                    pool.emplace_back(std::thread(&Std_pool::run, this, std::move(task)));
                    cur_thd_num++;
                    remain_tasks++;
                }else if(task_queue.size() < max_task_cap){
                    task_queue.push(std::move(task));
                    cv_awake.notify_one();
                    remain_tasks++;
                }else if(cur_thd_num < max_thd_num){
                    pool.emplace_back(std::thread(&Std_pool::run, this, std::move(task)));
                    cur_thd_num++;
                    remain_tasks++;
                }else {
                    // reject
                    drop_task++;
                    std::string msg = "The pool was full and " + std::to_string(drop_task)  + " tasks were dropped!";
                    throw msg;
                }
            }
            return func_ptr->get_future();
        }

        void wait_tasks(){
            uniq_guard guard(m_lock);
            cv_task_done.wait(guard, [this]{return !remain_tasks;});
        }

        void close(){
            {
                lock_guard guard(m_lock);
                stop = true;
                cv_awake.notify_all();
            }
            for (auto& t: pool) {
                t.join();
            }
        }

    private:
        int core_thd_num, max_thd_num, max_task_cap;
        int cur_thd_num = 0, drop_task = 0;
        std::queue<Task> task_queue;
        bool stop = false;
        std::vector<std::thread> pool;
        std::mutex m_lock;
        std::condition_variable cv_awake;
        std::condition_variable cv_task_done;
        std::atomic_uint remain_tasks = {0};

    private:
        void run(Task&& init_task){
            if(init_task.working()){
                init_task.call();
                remain_tasks--;
            }
            while(!stop){
                uniq_guard guard(m_lock);
                cv_awake.wait(guard, [this]{return stop || !task_queue.empty();});

                if(!stop){
                    Task curtask;
                    curtask = std::move(task_queue.front());
                    task_queue.pop();

                    curtask.call();
                    remain_tasks--;

                    guard.unlock();
                    
                    if(!remain_tasks){
                        lock_guard tmp_guard(m_lock);
                        cv_task_done.notify_one();
                    }
                }
            }
        }
    };

}