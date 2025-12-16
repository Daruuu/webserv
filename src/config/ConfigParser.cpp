#include "ConfigParser.hpp"
#include <fstream>
#include <iostream>

ConfigParser::ConfigParser() : serversCount_(0U)
{
}

ConfigParser::ConfigParser(const std::string& configFile)
{
	configFile_ = configFile;
	serversCount_ = 0U;
}

ConfigParser::~ConfigParser()
{
}

void ConfigParser::parse()
{
	if (validateExtensionAndPermissionsFile() == true)
	{
		std::cout << "\nwe can open the file: {" << configFile_ << "}\n";
	}
	else
	{
		std::cout << "\nError open file :(\n";
	}
}

const std::vector<ServerConfig>& ConfigParser::getServers() const
{
	return (servers_);
}

size_t ConfigParser::getServerCount() const
{
	return serversCount_;
}

//	============= PRIVATE CONSTRUCTORS ===============

ConfigParser::ConfigParser(const ConfigParser& other) :
	configFile_(other.configFile_), serversCount_(other.serversCount_),
	rawServerBlocks_(other.rawServerBlocks_)
{
	// servers_ = other.servers_;
}

ConfigParser& ConfigParser::operator=(const ConfigParser& other)
{
	if (this != &other)
	{
		ConfigParser tmp(other);
		std::swap(configFile_, tmp.configFile_);
		std::swap(serversCount_, tmp.serversCount_);
		// std::swap(servers_, tmp.servers_);
	}
	return *this;
}

bool ConfigParser::validateExtensionAndPermissionsFile() const
{
	if (configFile_.size() < 5 || configFile_.substr(configFile_.size() - 5) != ".conf")
	{
		//	throw exception
		return false;
	}
	//	validate exists
	std::cout << "file in validate: {" << configFile_.c_str() << "}\n";
	std::ifstream ifs(configFile_.c_str());
	if (!ifs.is_open())
	{
		//	throw config exception
		std::cout << "Cannot open config file: [" + configFile_ << "]";
		return false;
	}
	ifs.close();
	return true;
}

/*
bool ConfigParser::checkIfFileHasValidContent(const std::string& configFile) const
{
	return true;
}

void ConfigParser::extractBlocksOfEachServer(const std::string& file)
{
}

*/
