# Network Tests
add_executable(test_client
    test_network/test_client.cpp
    ../src/client/Client.cpp
    ../src/client/RequestProcessor.cpp
    ../src/http/HttpParser.cpp
    ../src/http/HttpRequest.cpp
    ../src/http/HttpResponse.cpp
    ../src/utils/StringUtils.cpp
)
target_include_directories(test_client PRIVATE ../src)

# CGI Tests
add_executable(test_cgi
    test_cgi/test_cgi_executor.cpp
    ../src/cgi/CgiExecutor.cpp
    ../src/http/HttpRequest.cpp
    ../src/http/HttpResponse.cpp
    ../src/utils/StringUtils.cpp
)
target_include_directories(test_cgi PRIVATE ../src)

# Add test target
add_custom_target(run_tests
    COMMAND test_client
    COMMAND test_cgi
    DEPENDS test_client test_cgi
)
