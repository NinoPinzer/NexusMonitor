cmake_minimum_required(VERSION 3.15)
project(Agent LANGUAGES CXX)

add_executable(Agent
        src/main.cpp

        src/AgentApp.hpp
        src/AgentApp.cpp
)

target_include_directories(Agent PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(Agent PRIVATE SysMonitorLib)

