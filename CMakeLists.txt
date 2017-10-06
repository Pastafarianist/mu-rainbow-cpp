cmake_minimum_required(VERSION 3.8)
project(cpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_COMPILER clang++)

set(SOURCE_FILES
        main.cpp
        utils.cpp utils.h
        storage.cpp storage.h
        )
add_executable(mu_roomy ${SOURCE_FILES})