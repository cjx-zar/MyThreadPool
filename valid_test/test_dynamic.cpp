#include "../dynamic_pool.h"


using namespace threadpool;

void plus_loop(int& x, int loop_num){
    // std::this_thread::yield();
    for(int i=0; i<loop_num; i++){
        x++;
        // std::cout << x << std::endl;
    }
}

void plus(int& x){
    x++;
}

void test_single_cpu(int thread_num, int loop_num){
    std::cout << "--------start testing dynamic_pool with single cpu--------" << std::endl;
    int cnt = 0;
    Dynamic_pool pool(thread_num);
    for(int i=0; i<loop_num; i++){
        pool.submit(plus, std::ref(cnt));
    }

    pool.wait_tasks();
    pool.close();
    if(cnt == loop_num) std::cout << "Success!" << std::endl;
    else std::cout << "cnt val:" << cnt << " Fuck all!" << std::endl;
}

void test_multi_cpu(int thread_num, int arrsize, int loop_num, bool enable_multicpu){ // 必须自行保证资源的独立性
    if(enable_multicpu)
        std::cout << "--------start testing dynamic_pool with multiple cpus opened--------" << std::endl;
    else
        std::cout << "--------start testing dynamic_pool with multiple cpus closed--------" << std::endl;
    int cnt = 0;
    Dynamic_pool pool(thread_num, enable_multicpu);
    std::vector<int> arr(arrsize, 0);
    for(size_t i=0; i<arr.size(); i++){
        pool.submit(plus_loop, std::ref(arr[i]), loop_num);
    }

    pool.wait_tasks();
    pool.close();
    for(size_t i=0; i<arr.size(); i++){
        if(arr[i] != loop_num) {
            std::cout << "cnt val:" << cnt << " Fuck all!" << std::endl;
            return;
        }
    }
    std::cout << "Success!" << std::endl; 
}

int main(){
    test_single_cpu(8, 10000);
    
    double t1 = tools::timewait(test_multi_cpu, 8, 10000, 10000, false);
    double t2 = tools::timewait(test_multi_cpu, 8, 10000, 10000, true);
    
    std::cout << "before opening multi-cpu running time : " << t1 << " (s)" << std::endl;
    std::cout << "after opening multi-cpu running time : " << t2 << " (s)" << std::endl;
    return 0;
}