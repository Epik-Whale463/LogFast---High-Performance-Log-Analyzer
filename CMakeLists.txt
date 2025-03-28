cmake_minimum_required(VERSION 3.10)
project(LogFast)

set(CMAKE_CXX_STANDARD 17)  # Use C++17

add_executable(logfast 
    src/main.cpp 
    src/log_generator.cpp 
    src/log_parser.cpp
)
