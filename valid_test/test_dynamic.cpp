#include "../dynamic_pool.h"

using namespace threadpool;

void plus(int& x, int num){
    // std::this_thread::yield();
    x += num;
}

int main(){
    std::cout << "--------start testing dynamic_pool--------" << std::endl;
    int thread_num = 8, loop_num = 10000;
    int cnt = 0;
    Dynamic_pool pool(thread_num);
    for(int i=0; i<loop_num; i++){
        pool.submit(plus, std::ref(cnt), 1);
    }

    pool.wait_tasks();
    pool.close();
    if(cnt == loop_num) std::cout << "Success!" << std::endl;
    else std::cout << "cnt val:" << cnt << " Fuck all!" << std::endl;
    return 0;
}