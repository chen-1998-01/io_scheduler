cmake_minimum_required(VERSION 2.8)
add_compile_options(
  "-std=c++11"
  "-g"
)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(
  SERVER_LIST
  ../src/boot/io_server.cpp
  ../src/boot/command.cpp
  ../src/net/address.cpp
  ../src/net/socket.cpp
  ../src/manager/memory_pool.cpp
  ../src/manager/times.cpp
  ../src/manager/file.cpp
  ../src/manager/logger.cpp
  ../src/concurrent/thread.cpp
  ../src/concurrent/coroutine.cpp
  ../src/concurrent/schedule.cpp
  ../src/concurrent/timer.cpp
  ../src/concurrent/event.cpp
  ../src/manager/handle.cpp
  ../src/io/io.cpp
  ../src/manager/hook.cpp
)
add_executable(io_server ${SERVER_LIST})
find_package(Threads)
target_link_libraries(io_server ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(io_server dl)

set(CMAKE_INSTALL_PREFIX /usr/local)
install(TARGETS io_server DESTINATION bin)