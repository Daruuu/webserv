#include "../test_utils.hpp"
#include "../../src/client/Client.hpp"
#include "../../src/config/ServerConfig.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>

void test_simple_get_request() {
    int fds[2];
    ASSERT_TRUE(socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == 0, 
                "socketpair creation");
    
    std::vector<ServerBlock> configs;
    ServerBlock config;
    config.port = 8080;
    LocationConfig loc;
    loc.path = "/";
    loc.root = "./www";
    loc.index = "index.html";
    loc.accepted_methods.insert("GET");
    config.locations.push_back(loc);
    configs.push_back(config);
    
    Client client(fds[0], &configs);
    
    const char* request = 
        "GET /index.html HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "\r\n";
    
    write(fds[1], request, strlen(request));
    
    client.handleRead();
    ASSERT_TRUE(client.needsWrite(), "Client needs to write after processing");
    
    client.handleWrite();
    
    char buffer[4096] = {0};
    ssize_t n = read(fds[1], buffer, sizeof(buffer));
    ASSERT_TRUE(n > 0, "Received response");
    
    std::string response(buffer, n);
    ASSERT_TRUE(response.find("HTTP/1.1") == 0, "Response starts with HTTP/1.1");
    ASSERT_TRUE(response.find("200") != std::string::npos || 
                response.find("404") != std::string::npos, 
                "Valid status code");
    
    close(fds[0]);
    close(fds[1]);
}

void test_post_request_with_body() {
    int fds[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
    
    std::vector<ServerBlock> configs;
    ServerBlock config;
    config.port = 8080;
    LocationConfig loc;
    loc.path = "/upload";
    loc.root = "./www";
    loc.accepted_methods.insert("POST");
    loc.upload_enabled = true;
    loc.upload_path = "/tmp";
    config.locations.push_back(loc);
    configs.push_back(config);
    
    Client client(fds[0], &configs);
    
    const char* request = 
        "POST /upload/test.txt HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Content-Length: 11\r\n"
        "\r\n"
        "Hello World";
    
    write(fds[1], request, strlen(request));
    
    client.handleRead();
    client.handleWrite();
    
    char buffer[4096] = {0};
    read(fds[1], buffer, sizeof(buffer));
    std::string response(buffer);
    
    ASSERT_TRUE(response.find("201") != std::string::npos || 
                response.find("200") != std::string::npos,
                "POST returns 200/201");
    
    close(fds[0]);
    close(fds[1]);
}

void test_keep_alive() {
    int fds[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
    
    std::vector<ServerBlock> configs;
    ServerBlock config;
    config.port = 8080;
    LocationConfig loc;
    loc.path = "/";
    loc.root = "./www";
    loc.accepted_methods.insert("GET");
    config.locations.push_back(loc);
    configs.push_back(config);
    
    Client client(fds[0], &configs);
    
    // First request
    const char* req1 = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
    write(fds[1], req1, strlen(req1));
    client.handleRead();
    client.handleWrite();
    
    char buf1[4096] = {0};
    read(fds[1], buf1, sizeof(buf1));
    
    ASSERT_TRUE(client.getState() == Client::READING || 
                client.getState() == Client::KEEP_ALIVE,
                "Client stays alive after first request");
    
    // Second request
    const char* req2 = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
    write(fds[1], req2, strlen(req2));
    client.handleRead();
    client.handleWrite();
    
    char buf2[4096] = {0};
    ssize_t n = read(fds[1], buf2, sizeof(buf2));
    
    ASSERT_TRUE(n > 0, "Second request succeeds on same connection");
    
    close(fds[0]);
    close(fds[1]);
}

int main() {
    test_simple_get_request();
    test_post_request_with_body();
    test_keep_alive();
    TEST_SUMMARY();
}
