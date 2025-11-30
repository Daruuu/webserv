#pragma once

#include <exception>
#include <string>
#include <unistd.h>
class ConfigException : public std::exception {
public:
	ConfigException(const std::string& message);

	virtual ~ConfigException() throw();

	virtual const char* what() const throw();
private:
	std::string message_;
	
	ConfigException();
	ConfigException& operator=(const ConfigException& other);
	ConfigException(const ConfigException& other);
};
