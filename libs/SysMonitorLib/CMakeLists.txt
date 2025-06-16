cmake_minimum_required(VERSION 3.15)
project(SysMonitorLib LANGUAGES CXX)

add_library(SysMonitorLib STATIC
        include/SysMonitor.hpp
        include/DataTypes.hpp
        src/PlatformUtils.hpp
        src/SysMonitor.cpp
        src/PlatformUtils_Linux.cpp
        src/PlatformUtils_Win.cpp
)

target_include_directories(SysMonitorLib
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if (WIN32)
    target_link_libraries(SysMonitorLib PRIVATE
            psapi
            user32
            pdh
            iphlpapi
            shlwapi
            advapi32
    )
elseif (UNIX)
    if (APPLE)
        # macOS-spezifische Bibliotheken hier
    else()
        # Linux-spezifische Bibliotheken hier
    endif()
endif()