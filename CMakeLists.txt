cmake_minimum_required(VERSION 3.26)
project(LinearHashing)

# -Wall -Wextra -Werror

add_compile_options(-O3 -std=c++17 -pedantic-errors -DPH2 -DETYPE=unsigned -DSIZE=1)

set(CMAKE_CXX_STANDARD 17)

add_executable(LinearHashing main.cpp
        ADS_set_temp.h
        )