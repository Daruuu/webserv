#ifndef WEBSERV_CONFIGPARSER_HPP
#define WEBSERV_CONFIGPARSER_HPP

#include <string>
#include <vector>

#include "ServerConfig.hpp"

class ConfigParser
{
public:
	ConfigParser();
	explicit ConfigParser(const std::string& configFile);
	~ConfigParser();

	//	Getters and Setters
	std::string& getConfigFilePath();
	size_t getServerCount() const;

	void parse() const;

	const std::vector<ServerConfig>& getServers() const;

private:
	std::string configFilePath_;
	size_t serversCount_;
	std::vector<std::string> rawServerBlocks_;
	std::vector<ServerConfig> servers_;

	//	constructors of copy and operator
	ConfigParser(const ConfigParser& other);
	ConfigParser& operator=(const ConfigParser& other);

	//	different validations.
	bool validateFileExtension() const;
	bool validateFilePermissions() const;
	bool validateBasicContent() const;
	bool validateCurlyBrackets() const;

	//	auxiliar member function
	void removeComments(std::string& line) const;
	void generatePrettyConfigLog() const;
	std::string trimLine(const std::string& line) const;
	std::string readFileContent() const;

	void MachineStatesOfConfigFile();
	void extractServerBlocks(const std::string& content);
	void parserServerBlocks() const;
	/**
	ServerConfig parseServerBlock(const std::string& block);
	LocationConfig parseLocationBlock(const std::string& block);
	*/
};

//ostream

#endif //WEBSERV_CONFIGPARSER_HPP
