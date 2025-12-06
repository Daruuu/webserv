#include "ServerManager.hpp"
#include <iostream>
#include <stdexcept>
#include <unistd.h>
#include <cstring>
#include <cerrno>

ServerManager::ServerManager(int port) 
: listener_(port) {

	epoll_.addFd(listener_.getFd(), EPOLLIN | EPOLLET);  // Edge-triggered for performance (default is LT: Level-triggered)

	listener_.listen();

	std::cout << "ServerManager initialized on port " << port << std::endl;
}

ServerManager::~ServerManager() {
	// Close all client connections
	for (std::vector<int>::iterator it = client_fds_.begin(); it != client_fds_.end(); ++it) {
		close(*it);
	}

	std::cout << "ServerManager shut down" << std::endl;
}

void ServerManager::run() {
	epoll_event events[MAX_EVENTS];

	std::cout << "Server started. Waiting for events..." << std::endl;

	while (true) {
		try {
			int num_events = epoll_.wait(events, MAX_EVENTS, -1);  // -1 = no timeout

			for (int i = 0; i < num_events; ++i) {
				int fd = events[i].data.fd;

				// Check for errors
				if (events[i].events & (EPOLLERR | EPOLLHUP)) {
					std::cerr << "Error on fd " << fd << std::endl;
					if (fd == listener_.getFd()) {
						throw std::runtime_error("Listener socket error");
					} else {
						handleClientDisconnect(fd);
					}
					continue;
				}

				// New connection on listener socket
				if (fd == listener_.getFd()) {
					handleNewConnection();
				} 
				// Data available on client socket
				else if (events[i].events & EPOLLIN) {
					handleClientData(fd);
				}
			}
		} catch (const std::exception& e) {
			std::cerr << "Error in event loop: " << e.what() << std::endl;
			break;
		}
	}
}

void ServerManager::handleNewConnection() {
	// Accept all pending connections (edge-triggered mode)
	while (true) {
		int client_fd = listener_.acceptConnection();
		if (client_fd == -1)  break;

		epoll_.addFd(client_fd, EPOLLIN | EPOLLET | EPOLLRDHUP);

		client_fds_.push_back(client_fd);

		std::cout << "Total clients: " << client_fds_.size() << std::endl;
	}
}

void ServerManager::handleClientData(int client_fd) {
	char buffer[4096];

	// Read all available data (edge-triggered mode)
	while (true) {
		ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);

		if (bytes_read > 0) {
			buffer[bytes_read] = '\0';
			std::cout << "\n=== Received " << bytes_read << " bytes from fd " << client_fd << " ===" << std::endl;
			std::cout << buffer << std::endl;
			std::cout << "=== End of data ===" << std::endl;

			// Send a simple response (we'll implement proper HTTP later)
			const char* response = 
				"HTTP/1.1 200 OK\r\n"
				"Content-Type: text/plain\r\n"
				"Content-Length: 34\r\n"
				"\r\n"
				"Hello from C++98 Web Server!\n";

			write(client_fd, response, strlen(response));

		} else if (bytes_read == 0) {
			// Connection closed by client
			std::cout << "Client " << client_fd << " closed connection" << std::endl;
			handleClientDisconnect(client_fd);
			break;
		} else {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				// No more data to read
				break;
			} else {
				std::cerr << "Error reading from client " << client_fd << ": " << strerror(errno) << std::endl;
				handleClientDisconnect(client_fd);
				break;
			}
		}
	}
}

void ServerManager::handleClientDisconnect(int client_fd) {
	epoll_.removeFd(client_fd);
	close(client_fd);

	for (std::vector<int>::iterator it = client_fds_.begin(); it != client_fds_.end(); ++it) {
		if (*it == client_fd) {
			client_fds_.erase(it);
			break;
		}
	}

	std::cout << "Client " << client_fd << " disconnected. Total clients: " 
		<< client_fds_.size() << std::endl;
}
