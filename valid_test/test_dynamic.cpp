#include "../dynamic_pool.h"

using namespace threadpool;

void plus_loop(int& x, int loop_num){
    // std::this_thread::yield();
    for(int i=0; i<loop_num; i++){
        x++;
    }
    
}

void plus(int& x, int num){
    x += num;
}


void test_single_cpu(int thread_num, int loop_num){
    std::cout << "--------start testing dynamic_pool with single cpu--------" << std::endl;
    int cnt = 0;
    Dynamic_pool pool(thread_num);
    for(int i=0; i<loop_num; i++){
        pool.submit(plus, std::ref(cnt), 1);
    }

    pool.wait_tasks();
    pool.close();
    if(cnt == loop_num) std::cout << "Success!" << std::endl;
    else std::cout << "cnt val:" << cnt << " Fuck all!" << std::endl;
}

void test_multi_cpu(int thread_num, int arrsize, int loop_num){ // 必须自行保证资源的独立性
    std::cout << "--------start testing dynamic_pool with multiple cpus--------" << std::endl;
    int cnt = 0;
    Dynamic_pool pool(thread_num, true);
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
    test_multi_cpu(8, 100, 10000);
    return 0;
}