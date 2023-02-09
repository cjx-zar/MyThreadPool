cmake_minimum_required(VERSION 3.10)

project(test)

set(CMAKE_CXX_FLAGS -std=c++11)

set(CMAKE_CXX_FLAGS -pthread)

add_definitions(
		"-Wall"
		"-g"
)

# add_executable(test test_fixthread.cpp)
add_executable(test test_stdpool.cpp)

# target_link_libraries(test -lpthread)
target_link_libraries(test -lpthread -lpqxx -lpq)