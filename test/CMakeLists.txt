cmake_minimum_required(VERSION 2.8)
add_compile_options(
  "-std=c++11"
  "-g"
)
set(
  SRC_LIST 
  ./concurrent_test.cpp
  ../src/concurrent/lock.cpp
  ../src/concurrent/thread.cpp
  ../src/concurrent/thread_pool.cpp
  ../src/manager/memory_pool.cpp
  ../src/manager/times.cpp
  ../src/manager/logger.cpp
  ../src/stream/file_stream.cpp
  )
add_executable(concurrent_test ${SRC_LIST})
find_package(Threads)
target_link_libraries(concurrent_test ${CMAKE_THREAD_LIBS_INIT})