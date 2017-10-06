cmake_minimum_required(VERSION 3.8)
project(cpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_COMPILER clang++)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEV -DDEBUG \
        -g3 -Wall -Wextra -pedantic -Wcast-align -Wcast-qual -Wconversion -Wctor-dtor-privacy \
        -Wdisabled-optimization -Wfloat-equal -Wformat=2 -Winit-self -Wlogical-op -Wmissing-include-dirs \
        -Wnoexcept -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-promo \
        -Wstrict-null-sentinel -Wstrict-overflow=5 -Wundef -Wno-unused -Wno-variadic-macros -Wno-parentheses \
        -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -fsanitize=address -fsanitize=undefined -fno-sanitize-recover \
        -fstack-protector -fdiagnostics-show-option")

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 \
        -DDEV -Wall -Wextra -pedantic -O2 -Wshadow -Wformat=2 -Wfloat-equal -Wconversion \
        -Wlogical-op -Wcast-qual -Wcast-align")

set(SOURCE_FILES
        main.cpp
        utils.cpp utils.h
        storage.cpp storage.h
        )
add_executable(mu_roomy ${SOURCE_FILES})