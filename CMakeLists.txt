cmake_minimum_required(VERSION 3.10)
project(webserv)

# ---------- global settings --------------------------------------------------
set(CMAKE_CXX_STANDARD 98)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------- mode selection ---------------------------------------------------
option(USE_MANUAL "Use manually specified source files" OFF)

if(USE_MANUAL)
    message(STATUS "MANUAL mode – using predefined source list")

    set(SOURCES
        src/main.cpp
        src/network/EpollWrapper.cpp
        src/network/TcpListener.cpp
        src/network/ServerManager.cpp
        # add more files here when needed
    )

else()
    message(STATUS "AUTO mode – discovering sources under src/")

    file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/src/*.cpp"
    )
endif()

# ---------- executable -------------------------------------------------------
add_executable(webserv ${SOURCES})

# ---------- include directories ---------------------------------------------
target_include_directories(webserv PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/cgi
    ${CMAKE_SOURCE_DIR}/src/client
    ${CMAKE_SOURCE_DIR}/src/config
    ${CMAKE_SOURCE_DIR}/src/http
    ${CMAKE_SOURCE_DIR}/src/network
    ${CMAKE_SOURCE_DIR}/src/utils
)
