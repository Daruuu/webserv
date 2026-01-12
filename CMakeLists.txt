cmake_minimum_required(VERSION 3.10)
project(webserv
        VERSION 0.1.0
        DESCRIPTION "HTTP Web Server"
        LANGUAGES CXX
)

# Global Configuration
set(CMAKE_CXX_STANDARD 98)
#set(CMAKE_CXX_STANDARD 11)  # use when testing
#set(CMAKE_CXX_EXTENSIONS OFF) # usa -std=c++98 en vez de -std=gnu++98
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

add_compile_options(-Wall -Wextra -Werror)

#include subdirectories of each module in src/
#include_directories(${CMAKE_SOURCE_DIR}/src)
add_subdirectory(src/config)

# testing
include_directories(${CMAKE_SOURCE_DIR}/lib)
# Si quieres activar tests (muy recomendado ponerlo opcional)
option(BUILD_TESTING "Build the testing tree." ON)

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests/unit)
endif()


# main executable
#        src/main.cpp
add_executable(webserv
        src/config/mainConfig.cpp
)

# enlazar con libreria.
target_link_libraries(webserv PRIVATE
        config
)

add_executable(Testing tests/unit/main_test.cpp)
