#pragma once
#include "./tools.h"
#include <thread>
#include <mutex>
#include <vector>
#include <queue>
#include <iostream>
#include <future>
#include <condition_variable>

namespace threadpool{
    using Task = tools::Task;
    using uniq_guard = std::unique_lock<std::mutex>;
    using lock_guard = std::lock_guard<std::mutex>; 
}