#include "ConfigParser.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "ConfigException.hpp"
#include "../common/namespaces.hpp"

ConfigParser::ConfigParser() : serversCount_(0)
{
}

ConfigParser::ConfigParser(const std::string& configFile) :
	configFilePath_(configFile), serversCount_(0U)
{
}

ConfigParser::~ConfigParser()
{
}

//	============= PRIVATE CONSTRUCTORS ===============

ConfigParser::ConfigParser(const ConfigParser& other) :
	configFilePath_(other.configFilePath_), serversCount_(other.serversCount_),
	rawServerBlocks_(other.rawServerBlocks_)
{
	// servers_ = other.servers_;
}

ConfigParser& ConfigParser::operator=(const ConfigParser& other)
{
	if (this != &other)
	{
		ConfigParser tmp(other);
		std::swap(configFilePath_, tmp.configFilePath_);
		std::swap(serversCount_, tmp.serversCount_);
		// std::swap(servers_, tmp.servers_);
	}
	return *this;
}

//	Getters and Setters

std::string& ConfigParser::getConfigFilePath()
{
	return configFilePath_;
}

size_t ConfigParser::getServerCount() const
{
	return serversCount_;
}

const std::vector<ServerConfig>& ConfigParser::getServers() const
{
	return servers_;
}

/**
 * todas las validaciones del archivo .conf
 * se gestionan en esta funcion.
 *
 */
void ConfigParser::parse() const
{
	if (!validateFileExtension())
	{
		throw ConfigException(
			config::errors::invalid_extension + configFilePath_);
	}
	if (!validateFilePermissions())
	{
		throw ConfigException(
			config::errors::cannot_open_file + configFilePath_);
	}
	if (!validateCurlyBrackets())
	{
		throw ConfigException(
			"Invalid number of curly brackets " + configFilePath_);
	}
	generatePrettyConfigLog();
	if (!validateBasicContent())
	{
		std::cout << "\nError open file :(\n";
	}
}

/**
 * manage if configFilePath:
 * has valid size of length
 * has extension '.conf'
 * is possible to open the file.
 * @return true or false
 */
bool ConfigParser::validateFileExtension() const
{
	if (configFilePath_.size() < 5 || configFilePath_.substr(
		configFilePath_.size() - 5) != config::paths::extension_file)
	{
		return false;
	}
	return true;
}

bool ConfigParser::validateFilePermissions() const
{
	std::ifstream ifs(configFilePath_.c_str());
	if (!ifs.is_open())
		return false;
	ifs.close();
	return true;
}

/**
 * check if lines start with comments '#'
 * iterate through each line of file.
 * @return
 */
bool ConfigParser::validateBasicContent() const
{
	std::ifstream ifs(configFilePath_.c_str());
	if (!ifs.is_open())
	{
		throw ConfigException(
			config::errors::cannot_open_file + configFilePath_ +
			" in validateBasicContent");
	}

	std::string line;
	size_t lineNumber = 0;

	//	buffer to save lines
	std::ostringstream logBuffer;

	while (std::getline(ifs, line))
	{
		++lineNumber;
		removeComments(line);
		line = trimLine(line);
		if (line.empty())
			continue;
		logBuffer << "|" << lineNumber << "|" << line << "\n";
	}
	ifs.close();
	return true;
}

bool ConfigParser::validateCurlyBrackets() const
{
	std::ifstream ifs(configFilePath_.c_str());
	if (!ifs.is_open())
	{
		throw ConfigException("Invalid number of curly brackets ");
	}
	std::string line;
	size_t openBracket = 0;
	size_t closeBracket = 0;
	while (std::getline(ifs, line))
	{
		removeComments(line);
		line = trimLine(line);
		if (line.empty())
			continue;
		if (line.find('{') != std::string::npos)
		{
			++openBracket;
		}
		else if (line.find('}') != std::string::npos)
		{
			++closeBracket;
		}
	}
	std::cout << "Total open brackets: " << openBracket <<
		"\nTotal open brackets: " << closeBracket << "\n";
	if (openBracket != closeBracket)
		return false;
	return true;
}

//	aux member functions
void ConfigParser::removeComments(std::string& line) const
{
	size_t commentPosition = line.find('#');
	if (commentPosition != std::string::npos)
	{
		line = line.substr(0, commentPosition);
	}
}

/**
 * export config file '.log'
 * remove empty lines and comment lines.
 */
void ConfigParser::generatePrettyConfigLog() const
{
	std::ifstream ifs(configFilePath_.c_str());
	if (!ifs.is_open())
	{
		throw ConfigException(
			config::errors::cannot_open_file +
			configFilePath_ +
			" (in generatePrettyConfigLog)"
		);
	}
	std::ofstream logFile(config::paths::log_file.c_str());
	if (!logFile.is_open())
	{
		// Aquí puedes elegir: lanzar excepción o solo warning
		std::cerr << "Warning: Could not open/create pretty log file: ";
		//<< config::paths::log_file_output << "\n";
		// throw ConfigException("Cannot write pretty log");
		return;
	}
	logFile << "=== Pretty print of configuration file ===\n";
	logFile << "File: " << configFilePath_ << "\n";
	logFile << "Generated: " << __DATE__ << " " << __TIME__ << "\n";
	logFile << "----------------------------------------\n\n";

	std::string line;

	// size_t lineNum = 0;
	while (std::getline(ifs, line))
	{
		// ++lineNum;
		removeComments(line);
		line = trimLine(line);
		if (line.empty())
			continue;
		//logFileOutput << lineNum << "|" << line << "\n";
		logFile << line << "\n";
	}
	ifs.close();
}

/**
 * remove includes: space, tab, newline and carriage return
 * 
 * @param line The string to trim
 * @return New string without leading/trailing whitespace
 * 
 * Examples:
 *   "  hello  " -> "hello"
 *   "\t\ntest\r\n" -> "test"
 *   "   " -> ""
 */
std::string ConfigParser::trimLine(const std::string& line) const
{
	const std::string whitespace = " \t\n\r";

	const size_t start = line.find_first_not_of(whitespace);
	if (start == std::string::npos)
	{
		return "";
	}
	const size_t end = line.find_last_not_of(whitespace);
	return line.substr(start, end - start + 1);
}

/**
 * Reads entire content of config file into a string.
 * @return String containing all file content
 * @throws ConfigException if file cannot be opened
 */
std::string ConfigParser::readFileContent() const
{
	std::ifstream file(configFilePath_.c_str());

	if (!file.is_open())
	{
		throw ConfigException("Cannot open config file: " + configFilePath_);
		// return "Cannot open config file: ";
	}
	// Read entire file using stringstream
	/**
	std::stringstream buffer;
	buffer << file.rdbuf();
	while (buffer <<  )
	{

	}
	file.close();
	return buffer.str();
	*/
	return "";
}

/**
 * la idea es que dependiendo de que estado se encuentre se actualize el enum,
 * asi saber cuando esta en un bloque de server o location o fuera de bloque
 *
 */
//	TODO: reorganize this part to understand whar we need to do

/*
void ConfigParser::MachineStatesOfConfigFile()
{
	config::ParserState state = config::OUTSIDE_BLOCK;
	size_t braceCount = 0;
	size_t countLines = 0;

	std::ifstream ifs(configFilePath_.c_str());
	if (!ifs.is_open())
	{
		std::ostringstream oss;
		oss << config::errors::cannot_open_file << configFilePath_ <<
			" in MachineStatesOfConfigFile()";
		throw ConfigException(oss.str());
	}

	std::string line;
	config::ParserState state;

	while (getline(ifs, line))
	{
		++countLines;

		removeComments(line);
		line = trimLine(line);

		if (line.empty())
			continue ;
	}
}
*/

/**
 * Extracts all server { } blocks from the config content.
 * Handles nested braces within location blocks.
 * @param content The entire config file content
 */
void ConfigParser::extractServerBlocks(const std::string& content)
{
	size_t pos = 0;

	while ((pos = content.find("server", pos)) != std::string::npos)
	{
		// Find opening brace
		size_t braceStart = content.find('{', pos);
		if (braceStart == std::string::npos)
			break;

		// Find matching closing brace (handle nested braces)
		int braceCount = 1;
		size_t braceEnd = braceStart + 1;

		while (braceEnd < content.size() && braceCount > 0)
		{
			if (content[braceEnd] == '{')
				braceCount++;
			else if (content[braceEnd] == '}')
				braceCount--;
			braceEnd++;
		}

		// Extract the complete server block
		std::string block = content.substr(pos, braceEnd - pos);
		rawServerBlocks_.push_back(block);

		pos = braceEnd;
	}

	serversCount_ = rawServerBlocks_.size();
}

void ConfigParser::parserServerBlocks() const
{
	// TODO: Implement when ServerConfig is ready
	// For now, just count them
	std::cout << "Found " << serversCount_ << " server block(s)" << std::endl;
}
