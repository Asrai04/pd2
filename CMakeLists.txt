cmake_minimum_required(VERSION 3.30)
project(pd2)

set(CMAKE_CXX_STANDARD 20)

add_executable(pd2 main.cpp
        MemoryManager.cpp
        MemoryManager.h
        BlockMemory.cpp
        BlockMemory.h
)

target_link_libraries(pd2 ws2_32)
