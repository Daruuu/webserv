#pragma once

#include "EpollWrapper.hpp"
#include "TcpListener.hpp"
#include <map>
#include <vector>

class ServerManager {
public:
    ServerManager(int port);
    ~ServerManager();
    
    // Run the server main loop
    void run();
    
private:
    EpollWrapper epoll_;
    TcpListener listener_;
    
    // Maximum number of events to process at once
    static const int MAX_EVENTS = 64;
    
    // Client file descriptors
    std::vector<int> client_fds_;
    
    // Disable copying
    ServerManager(const ServerManager&);
    ServerManager& operator=(const ServerManager&);
    
    // Event handlers
    void handleNewConnection();
    void handleClientData(int client_fd);
    void handleClientDisconnect(int client_fd);
};
