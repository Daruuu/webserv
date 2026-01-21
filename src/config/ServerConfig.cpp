#include "ServerConfig.hpp"

#include <iostream>

ServerConfig::ServerConfig() : listen_port_(0), max_body_size_(0)
{
}

ServerConfig::ServerConfig(const ServerConfig& other) :
	listen_port_(other.listen_port_), host_address_(other.host_address_),
	server_name_(other.server_name_), max_body_size_(0),
	error_pages_(other.error_pages_)
{
}

ServerConfig& ServerConfig::operator=(const ServerConfig& other)
{
	if (this != &other)
	{
		listen_port_ = other.listen_port_;
		host_address_ = other.host_address_;
		server_name_ = other.server_name_;
		max_body_size_ = other.max_body_size_;
		error_pages_ = other.error_pages_;
	}
	return *this;
}

ServerConfig::~ServerConfig()
{
}

void ServerConfig::print() const
{
	std::cout << "Print of info in ServerConfig" ;
}
