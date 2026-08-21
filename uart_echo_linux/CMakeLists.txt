cmake_minimum_required(VERSION 3.16)

project(MyProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(UART_CLIENT_SOURCES
        src/uart_lib.hpp
        src/uart_client.cpp
)

add_executable(uart-client src/uart_client.cpp)
