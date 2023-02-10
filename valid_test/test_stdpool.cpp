#include "../std_pool.h"
#include <pqxx/pqxx>


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

void test_db_seq(int loop_num){
    std::cout << "--------start testing normal sequential execute--------" << std::endl;

    pqxx::connection conn("dbname=credit user=postgres password=chen4 hostaddr=127.0.0.1 port=5432");
    std::string sql_str = "select count(*) from previous cross join (select * from application limit 10) as tmp;";
    pqxx::nontransaction worker(conn);
    for(int i=0; i<loop_num; i++){
        worker.exec(sql_str);
    }
    conn.disconnect();
}

void test_db_multithd(int loop_num, bool safe){
    if(safe)
        std::cout << "--------start testing std_pool with safe resource--------" << std::endl;
    else
        std::cout << "--------start testing std_pool with unsafe resource--------" << std::endl;

    const std::string sql_str = "select count(*) from previous cross join (select * from application limit 10) as tmp;";
    const std::string tmp = "";
    using type = pqxx::result(pqxx::nontransaction::*)(const std::string&, const std::string&); //有重载在bind时必须指明函数
    Std_pool pool("CPU", INT_MAX, safe);
    std::vector<pqxx::connection*> conn_pool;
    std::vector<std::future<pqxx::result>> future_pool;
    std::vector<pqxx::nontransaction*> worker_pool; //用指针防止使用deleted的拷贝构造函数
    for(int i=0; i<loop_num; i++){
        //多个worker和多个conn保证对数据库的高效访问（模拟连接池）
        conn_pool.push_back(new pqxx::connection("dbname=credit user=postgres password=chen4 hostaddr=127.0.0.1 port=5432"));
        worker_pool.push_back(new pqxx::nontransaction(*conn_pool[i]));
        future_pool.push_back(pool.submit((type)&pqxx::nontransaction::exec, worker_pool[i], sql_str, tmp)); 
    }
    pool.wait_tasks();
    pool.close();
    // for(auto&& fut : future_pool){
    //     std::cout << fut.get().begin()[0] << std::endl;
    // }
    for(auto&& conn_ptr : conn_pool){
        (*conn_ptr).disconnect();
    }
    
    // conn.disconnect();
}

int main(){
    // test_single_cpu(10000);
    // double t1 = tools::timewait(test_db_seq, 100);
    // std::cout << "Sequential running time : " << t1 << " (s)" << std::endl;

    // double t2 = tools::timewait(test_db_multithd, 100, false);
    // std::cout << "Multi-thread running time : " << t2 << " (s)" << std::endl;

    double t3 = tools::timewait(test_db_multithd, 100, true);
    std::cout << "Multi-thread running time : " << t3 << " (s)" << std::endl;
    return 0;
}