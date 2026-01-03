cmake_minimum_required(VERSION 4.1)
project(2048 C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(2048
        main_process.c
        common.h)
