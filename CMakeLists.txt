cmake_minimum_required(VERSION 3.28)
project(monop)

set(CMAKE_CXX_STANDARD 23)

add_executable(monop main.cpp
        player.cpp
        board.cpp
        dice.cpp
        dice.h

)

find_package(fmt REQUIRED)
target_link_libraries(monop fmt::fmt)
