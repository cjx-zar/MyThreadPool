# MyThreadPool
A thread pool interface implement by C++

# How to use ThreadPool
**fixthread_pool.h** is a simple thread pool with a single task queue. It creates a pool of a given number of threads, each of which dynamically fetches and executes tasks from the task queue. When the user can ensure that the resource is multithreaded safe (for example, multiple read tasks), the thread pool will bind the threads to different cpus whenever possible (optionally specified) to improve the pool's efficiency.

**std_pool.h** is the upgrade version of **fixthread_pool.h**. It supports more kinds of task scheduling, such as member functions, base class member functions, and so on. It allows users to set the number of core threads, the maximum number of threads, the blocking queue capacity, the rejection policy according to the characteristics of the JAVA thread pool. Likewise, it supports multi-threaded parallelism when resources are safe.

# Validity test demos
There are two Demos for testing **fixthread_pool.h** and **std_pool.h** in folder **valid_test**. My compilation options can be viewed in **CMakeLists.txt**. To run it,
you can use the following command:
```
cd ./valid_test/build
cmake ..
make
./test
```

# Functional test
| Method | Task Description | Running time |
| :----: |      :----:      |    :----:    |
| Sequential execution | 10 $\times$ count for 15 million rows with pgsql |  7.19s  |
| Sequential execution | 50 $\times$ count for 15 million rows with pgsql |  35.71s  |
| Sequential execution | 100 $\times$ count for 15 million rows with pgsql | 71.06s  |
| Std_pool with safe resources | 10 $\times$ count for 15 million rows with pgsql |  0.87s  |
| Std_pool with safe resources | 50 $\times$ count for 15 million rows with pgsql |  1.98s  |
| Std_pool with safe resources | 100 $\times$ count for 15 million rows with pgsql | 3.15s  |



