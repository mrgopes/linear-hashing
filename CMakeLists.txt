cmake_minimum_required(VERSION 3.26)
project(LinearHashing)

# -Wall -Wextra -Werror

add_compile_options(-std=c++17 -DPH2 -DETYPE=unsigned -DSIZE=1)

set(CMAKE_CXX_STANDARD 17)

add_executable(LinearHashing main.cpp
        ADS_set_temp.h
        )