cmake_minimum_required(VERSION 4.0)
project(webserv)

set(CMAKE_CXX_STANDARD 98)

# Source files
set(SOURCES
	src/main.cpp
	src/network/EpollWrapper.cpp
	src/network/TcpListener.cpp
	src/network/ServerManager.cpp
)

add_executable(webserv ${SOURCES})
