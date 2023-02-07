#include "../std_pool.h"


using namespace threadpool;

void plus_loop(int& x, int loop_num){
    // std::this_thread::yield();
    for(int i=0; i<loop_num; i++){
        x++;
    }
}

void plus(int& x){
    // std::this_thread::yield();
    x++;
    // std::cout << x << std::endl;
}

void test_single_cpu(int loop_num){
    std::cout << "--------start testing std_pool--------" << std::endl;
    int cnt = 0;
    Std_pool pool("CPU");
    for(int i=0; i<loop_num; i++){
        try {
            pool.submit(plus, std::ref(cnt));
        }catch (const std::string& msg){
            std::cerr << msg << std::endl;
        }
        
    }

    pool.wait_tasks();
    pool.close();
    if(cnt == loop_num) std::cout << "Success!" << std::endl;
    else std::cout << "cnt val:" << cnt << " Fuck all!" << std::endl;
}

int main(){
    test_single_cpu(10000);
    
    return 0;
}