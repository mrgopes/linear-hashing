cmake_minimum_required(VERSION 3.26)
project(LinearHashing)

add_compile_options(-Wall -Wextra -Werror -O3 -std=c++17 -pedantic-errors -DPH2 -DETYPE=unsigned -DSIZE=1)

set(CMAKE_CXX_STANDARD 17)

add_executable(LinearHashing simpletest.cpp
        ADS_set_temp.h
        )