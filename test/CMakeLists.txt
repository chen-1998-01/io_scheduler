cmake_minimum_required(VERSION 2.8)
add_compile_options(
  "-std=c++11"
  "-g"
)


set(
  NET_LIST
  ./inet_test.cpp
  ../../src/net/address.cpp
  ../../src/net/socket.cpp
  ../../src/manager/memory_pool.cpp
  ../../src/manager/times.cpp
  ../../src/manager/file.cpp
  ../../src/manager/logger.cpp
  ../../src/concurrent/thread.cpp
  ../../src/concurrent/coroutine.cpp
  ../../src/concurrent/schedule.cpp
  ../../src/concurrent/timer.cpp
  ../../src/concurrent/event.cpp
  ../../src/manager/handle.cpp
  ../../src/io/io.cpp
  ../../src/manager/hook.cpp
)
add_executable(inet_test ${NET_LIST})
find_package(Threads)
target_link_libraries(inet_test ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(inet_test dl)

set(
  CLIENT
  ./client_test.cpp
  ../../src/net/address.cpp
  ../../src/net/socket.cpp
  ../../src/manager/memory_pool.cpp
  ../../src/manager/times.cpp
  ../../src/manager/file.cpp
  ../../src/manager/logger.cpp
  ../../src/concurrent/thread.cpp
  ../../src/concurrent/coroutine.cpp
  ../../src/concurrent/schedule.cpp
  ../../src/concurrent/timer.cpp
  ../../src/concurrent/event.cpp
  ../../src/manager/handle.cpp
  ../../src/io/io.cpp  
  ../../src/manager/hook.cpp
)
add_executable(client_test ${CLIENT})
find_package(Threads)
target_link_libraries(client_test ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(client_test dl)

set(
  EXECUTABLE_OUTPUT_PATH
  ${PROJECT_BINARY_DIR}/bin
)

