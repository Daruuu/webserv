cmake_minimum_required(VERSION 3.10)
project(webserv
        VERSION 0.1.0
        DESCRIPTION "HTTP Web Server"
        LANGUAGES CXX
)

# Global Configuration
set(CMAKE_CXX_STANDARD 98)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # usa -std=c++98 en vez de -std=gnu++98

set(CMAKE_COMPILE_WARNING_AS_ERROR ON)
add_compile_options(-Wall -Wextra -Werror)

# Add all modules src/
add_subdirectory(src/cgi)
add_subdirectory(src/client)
add_subdirectory(src/common)
add_subdirectory(src/config)
add_subdirectory(src/http)
add_subdirectory(src/network)
add_subdirectory(src/utils)

# --- main executable ---

add_executable(webserv
        src/main.cpp
)

# link libs to executable.
target_link_libraries(webserv PRIVATE
        network
        client
        cgi
        http
        config
        utils
        common
)

#include_directories(${CMAKE_SOURCE_DIR}/lib)
# testing (Optional)
option(BUILD_TESTING "Build the testing tree." ON)
if(BUILD_TESTING)
        enable_testing()
        add_subdirectory(tests/unit)
endif()

#add_executable(Testing tests/unit/main_test.cpp)
# Si quieres que el binario de los tests se cree,
# tenemos que incluir la carpeta donde está tu código
#add_subdirectory(src/http)
add_subdirectory(tests/test_http)
add_subdirectory(tests/manual_processor)
add_subdirectory(tests/manual_client)


