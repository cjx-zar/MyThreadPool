#pragma once
#include "./header.h"

namespace threadpool{
    class Dynamic_pool{
    public:
        Dynamic_pool(int num): thread_num(num){
            lock_guard guard(m_lock);
            for(int i=0; i<thread_num; i++){
                pool.emplace_back(std::thread(&Dynamic_pool::run, this, i));
            }
        }

        Dynamic_pool(int num, bool enable_multicpu, int cpus = 4): thread_num(num), enable_multicpu(enable_multicpu){
            lock_guard guard(m_lock);
            CPU_ZERO(&cpuset);

            for(int i=0; i<cpus; i++){
                CPU_SET(i, &cpuset);
            }
            for(int i=0; i<thread_num; i++){
                pool.emplace_back(std::thread(&Dynamic_pool::run, this, i));
            }
        }

        ~Dynamic_pool(){
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
                task_queue.push(std::move(task));
                remain_tasks++;
                cv_awake.notify_one();
            }
            return func_ptr->get_future();
        }
        
        bool is_free(){
            return !remain_tasks;
        }

        void wait_tasks(){
            uniq_guard guard(m_lock);
            cv_task_done.wait(guard, [this]{return is_free();});
        }

        int tasks_left(){
            return remain_tasks;
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
        int thread_num;
        int remain_tasks = 0;
        cpu_set_t cpuset;
        bool stop = false;
        bool enable_multicpu = false;
        std::vector<std::thread> pool;
        std::mutex m_lock;
        std::condition_variable cv_awake;
        std::condition_variable cv_task_done;
        std::queue<Task> task_queue;
    
    private:    
        int push(std::thread&& thd){
            pool.push_back(std::move(thd));
            return (int)pool.size() - 1;
        }

        void run(int idx){
            int rc =pthread_setaffinity_np(pool[idx].native_handle(), sizeof(cpu_set_t), &cpuset);
            if (rc != 0) {
                std::cerr << "Error calling pthread_setaffinity_np: " << rc << std::endl;
            }
            
            while(!stop){
                uniq_guard guard(m_lock);
                cv_awake.wait(guard, [this]{return stop || !is_free();});

                if(!stop){
                    Task curtask;
                    if(!task_queue.empty()){
                        curtask = std::move(task_queue.front());
                        task_queue.pop();
                    }

                    if(enable_multicpu)
                        guard.unlock();
                    
                    if(curtask.working()){
                        curtask.call();
                        remain_tasks--;
                    }
                    
                    if(!enable_multicpu)
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