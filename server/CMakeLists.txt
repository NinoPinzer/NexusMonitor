cmake_minimum_required(VERSION 3.15)
project(Server LANGUAGES CXX)

add_executable(Server
        src/main.cpp
)

target_include_directories(Server PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

find_package(SQLite3 REQUIRED)
target_link_libraries(Server PRIVATE SQLite::SQLite3)

