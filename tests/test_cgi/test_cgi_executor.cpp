#include "../test_utils.hpp"
#include "../../src/cgi/CgiExecutor.hpp"
#include "../../src/http/HttpRequest.hpp"
#include "../../src/http/HttpResponse.hpp"
#include <cstdlib>

void test_simple_cgi_execution() {
    CgiExecutor executor;
    HttpRequest request;
    
    request.setMethod("GET");
    request.setUri("/cgi-bin/hello.py");
    request.setVersion("HTTP/1.1");
    request.setHeader("host", "localhost");
    
    // Make script executable
    system("chmod +x tests/test_cgi/scripts/hello.py");
    
    HttpResponse response = executor.execute(
        request,
        "tests/test_cgi/scripts/hello.py",
        "/usr/bin/python3"
    );
    
    ASSERT_EQ(response.getStatus(), 200, "CGI returns 200");
    
    std::string body = response.getBody();
    ASSERT_TRUE(body.find("Hello from CGI!") != std::string::npos,
                "CGI body contains expected text");
}

void test_cgi_environment_variables() {
    CgiExecutor executor;
    HttpRequest request;
    
    request.setMethod("GET");
    request.setUri("/cgi-bin/echo_env.py?key=value&foo=bar");
    request.setVersion("HTTP/1.1");
    request.setHeader("host", "localhost");
    
    system("chmod +x tests/test_cgi/scripts/echo_env.py");
    
    HttpResponse response = executor.execute(
        request,
        "tests/test_cgi/scripts/echo_env.py",
        "/usr/bin/python3"
    );
    
    std::string body = response.getBody();
    
    ASSERT_TRUE(body.find("REQUEST_METHOD=GET") != std::string::npos,
                "REQUEST_METHOD set correctly");
    ASSERT_TRUE(body.find("QUERY_STRING=key=value&foo=bar") != std::string::npos,
                "QUERY_STRING set correctly");
}

void test_cgi_post_with_body() {
    CgiExecutor executor;
    HttpRequest request;
    
    request.setMethod("POST");
    request.setUri("/cgi-bin/echo_stdin.py");
    request.setVersion("HTTP/1.1");
    request.setHeader("content-type", "application/x-www-form-urlencoded");
    request.setBody("name=test&value=123");
    
    // Create echo script
    system("echo '#!/usr/bin/env python3\nimport sys\nprint(\"Content-Type: text/plain\")\nprint(\"\")\nprint(sys.stdin.read())' > tests/test_cgi/scripts/echo_stdin.py");
    system("chmod +x tests/test_cgi/scripts/echo_stdin.py");
    
    HttpResponse response = executor.execute(
        request,
        "tests/test_cgi/scripts/echo_stdin.py",
        "/usr/bin/python3"
    );
    
    std::string body = response.getBody();
    ASSERT_TRUE(body.find("name=test&value=123") != std::string::npos,
                "CGI receives POST body on stdin");
}

void test_cgi_timeout() {
    CgiExecutor executor;
    HttpRequest request;
    
    request.setMethod("GET");
    request.setUri("/cgi-bin/timeout.py");
    request.setVersion("HTTP/1.1");
    
    system("chmod +x tests/test_cgi/scripts/timeout.py");
    
    HttpResponse response = executor.execute(
        request,
        "tests/test_cgi/scripts/timeout.py",
        "/usr/bin/python3"
    );
    
    ASSERT_TRUE(response.getStatus() == 504 || response.getStatus() == 500,
                "Timeout returns 504 or 500");
}

int main() {
    // Create test directory
    system("mkdir -p tests/test_cgi/scripts");
    
    test_simple_cgi_execution();
    test_cgi_environment_variables();
    test_cgi_post_with_body();
    test_cgi_timeout();
    
    TEST_SUMMARY();
}
