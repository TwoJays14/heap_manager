cmake_minimum_required(VERSION 3.28)
project(heapManager C)

set(CMAKE_C_STANDARD 23)

add_executable(heapManager main.c
        heap_manager.h
        heap_manager.c
)
